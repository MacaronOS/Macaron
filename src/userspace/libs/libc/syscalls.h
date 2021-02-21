#ifndef __libc_syscalls
#define __libc_syscalls

#define SYS_PUTC 0
#define SYS_EXIT 1
#define SYS_FORK 2

int putc(char c);
int exit(int error_code);
int fork();

#endif // __libc_syscalls

