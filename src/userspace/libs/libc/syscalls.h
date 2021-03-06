#ifndef __libc_syscalls
#define __libc_syscalls

#define SYS_PUTC 0
#define SYS_EXIT 1
#define SYS_FORK 2
#define SYS_EXECVE 9
#define SYS_MMAP 90

int putc(char c);
int exit(int error_code);
int fork();
int execve(const char* filename, const char* const* argv, const char* const* envp);

#define MAP_SHARED 0x01
#define MAP_PRIVATE 0x02
#define MAP_FIXED 0x10
#define MAP_ANONYMOUS 0x20
#define MAP_STACK 0x40
#define MAP_NORESERVE 0x80

#define PROT_READ 0x1
#define PROT_WRITE 0x2
#define PROT_EXEC 0x4
#define PROT_NONE 0x0 

struct mmap_params {
    void* start;
    unsigned int length;
    int prot;
    int flags;
    int fd;
    unsigned int offset;
};

void* mmap(void *start, unsigned int length, int prot , int flags, int fd, unsigned int offset);

#endif // __libc_syscalls
