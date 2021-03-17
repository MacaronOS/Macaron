#include "syscalls.h"

static inline int do_syscall(int num, int arg1, int arg2, int arg3, int arg4, int arg5)
{
    int a;
    asm volatile("int $0x80"
                 : "=a"(a)
                 : "0"(num), "b"(arg1), "c"(arg2), "d"(arg3), "S"(arg4), "D"(arg5));
    return a;
}

int putc(char c)
{
    return do_syscall(SYS_PUTC, c, 0, 0, 0, 0);
}

int exit(int error_code)
{
    return do_syscall(SYS_EXIT, error_code, 0, 0, 0, 0);
}

int fork()
{
    return do_syscall(SYS_FORK, 0, 0, 0, 0, 0);
}

int open(const char* filename, int flags, unsigned short mode)
{
    return do_syscall(SYS_OPEN, filename, flags, mode, 0, 0);
}

int execve(const char* filename, const char* const* argv, const char* const* envp)
{
    return do_syscall(SYS_EXECVE, filename, argv, envp, 0, 0);
}

void* mmap(void* start, unsigned int length, int prot, int flags, int fd, unsigned int offset)
{
    volatile struct mmap_params params;
    params.start = start;
    params.length = length;
    params.prot = prot;
    params.flags = flags;
    params.fd = fd;
    params.offset = offset;
    return do_syscall(SYS_MMAP, &params, 0, 0, 0, 0);
}
