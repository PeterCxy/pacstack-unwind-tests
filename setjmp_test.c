#include <stdlib.h>
#include <stdio.h>
#include "setjmp.h"

void bad(jmp_buf env) {
    int should_crash = 0;
    printf("hello! I'm the bad function\n");
    printf("enter 1 for me to crash (longjmp): ");
    scanf("%d", &should_crash);
    if (should_crash)
        longjmp(env, -1);
    printf("thank you :D\n");
}

int main() {
    jmp_buf env;
    int a = 0;
    if (setjmp(env)) {
        printf("exception!\n");
        a = 1;
    } else {
        bad(env);
    }
    printf("ending %d\n", a);
    return 0;
}
