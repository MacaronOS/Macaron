#include "syscalls.h"

static inline int do_syscall(int intNum, int arg1, int arg2, int arg3, int arg4, int arg5)
{
    int a;
    asm volatile("int $0x80" : "=a" (a) : "0" (intNum), "b" (arg1), "c" (arg2), "d" (arg3), "S" (arg4), "D" (arg5));
    return a;
}

int putc(char c)
{
    return do_syscall(SYS_PUTC, c, 0, 0, 0, 0);
}