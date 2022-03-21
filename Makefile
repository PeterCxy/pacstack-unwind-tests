CC := ../pacstack-llvm/build/clang/bin/clang
CXX := ../pacstack-llvm/build/clang/bin/clang++
CXXFLAGS ?= -march=armv8.3-a -nostdinc++ -nostdlib++ -I ../pacstack-llvm/libunwind/include -L ../pacstack-llvm/build/runtimes/lib -Wl,-rpath,../pacstack-llvm/build/runtimes/lib -lc++ -mllvm -pacstack=full
# clang does not enable unwind tables (Dwarf / EHABI) for C by default.
# Enable it with -funwind-tables so we can use libunwind
CFLAGS ?= -funwind-tables -march=armv8.3-a -I ../pacstack-llvm/libunwind/include -L ../pacstack-llvm/build/runtimes/lib -Wl,-rpath,../pacstack-llvm/build/runtimes/lib -lunwind -mllvm -pacstack=full

setjmp_test.o: setjmp.o

setjmp_test: setjmp_test.o
	$(CC) $(CFLAGS) -o setjmp_test setjmp.o setjmp_test.o

confirm_exception: confirm_exception.o
	$(CXX) $(CXXFLAGS) -o confirm_exception confirm_exception.o

simple: simple.o
	$(CXX) $(CXXFLAGS) -o simple simple.o

clean:
	rm -rf *.o
	rm -rf setjmp_test
	rm -rf test
	rm -rf confirm_exception
