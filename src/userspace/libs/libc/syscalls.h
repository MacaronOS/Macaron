#ifndef __libc_syscalls
#define __libc_syscalls

#define SYS_PUTC 0
#define SYS_EXIT 1
#define SYS_FORK 2
#define SYS_EXECVE 9

int putc(char c);
int exit(int error_code);
int fork();
int execve(const char* filename, const char* const* argv, const char* const* envp);

#endif // __libc_syscalls
