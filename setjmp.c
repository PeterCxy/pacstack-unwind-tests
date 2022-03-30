#include <stdio.h>
#include <stdlib.h>
#include <libunwind.h>

typedef void *jmp_buf[3];

__attribute__ ((naked))
int _longjmp_return(int val, unsigned long cr, void *target_addr) {
    // Simulate the PACStack epilogue of the original _setjmp function
    __asm volatile (
        "mov x30, x2\n"
        "mov x28, x1\n"
        "mov x15, xzr\n"
        "pacia x15, x28\n"
        "eor x30, x30, x15\n"
        "autia x30, x28\n"
        "br x30\n"
    );
}

void longjmp(jmp_buf env, int val) {
    unw_context_t uc;
    unw_cursor_t cursor;
    unw_getcontext(&uc);
    unw_init_local(&cursor, &uc);

    struct unw_pacstack_info info;
    unwind_pacstack_init(&info);

    unw_word_t sp, fp;

    do {
        if (unw_get_reg(&cursor, UNW_REG_SP, &sp) < 0) exit(1);

        // PACStack -- verify the call stack while we are attempting longjmp
        // so that we must jump to somewhere *on the call stack*
        if (unwind_pacstack_verify_frame(&cursor, &info) != _URC_NO_REASON) {
            printf("PACStack error while attempting longjmp, aborting.\n");
            exit(1);
        }

        if (sp != (unw_word_t) env[0]) continue;

        // Redirect execution into _longjmp_return
        // This is used to simulate a return from _setjmp
        unw_set_reg(&cursor, UNW_ARM64_X2, (unw_word_t) env[1]);
        unw_set_reg(&cursor, UNW_ARM64_X1, (unw_word_t) env[2]);
        unw_set_reg(&cursor, UNW_ARM64_X0, val);
        unw_set_reg(&cursor, UNW_REG_IP, (unw_word_t) (void *) _longjmp_return);

        unw_resume(&cursor);
    } while (unw_step(&cursor) > 0);
}

__attribute__ ((noinline))
void dummy() {
    // Do not optimize me away plz
    __asm volatile ("");
}

int __setjmp(jmp_buf env, unsigned long sp, unsigned long cr) {
    // LLVM might decide here that we do not need PACStack
    // because this function does no further function calls
    // Prevent that by just calling a dummy function
    // because we actually want the PACStack stuff and verify
    // it when we longjmp() back
    dummy();

    // __builtin_frame_address uses the frame record, which is different
    // from the actual stack frame addresses. Use the real sp acquired
    // from the setjmp wrapper here.
    env[0] = (void*) sp;
    env[1] = __builtin_return_address(0);
    env[2] = (void*) cr;

    return 0;
}

__attribute__ ((naked))
int setjmp(jmp_buf env) {
    // Wrapper over _setjmp, exposes the original x28 (CR) and SP from caller
    // as the second parameter so that we can store then
    // It is import to have this function as naked so it doesn't
    // create an extra stack frame (such that the compiler won't spill x28
    // onto the stack and calculate a new one)
    __asm volatile (
        "mov x1, sp\n"
        "mov x2, x28\n"
        "br %[__setjmp]"
        :: [__setjmp] "r" (&__setjmp)
    );
}
