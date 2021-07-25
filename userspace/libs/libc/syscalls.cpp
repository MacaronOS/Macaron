#include "syscalls.hpp"

#include <wisterialib/common.hpp>
#include <wisterialib/posix/defines.hpp>
#include <wisterialib/posix/shared.hpp>

#define ToSysArg(arg) ((int)(arg))

static inline int do_syscall(Syscall num, int arg1 = 0, int arg2 = 0, int arg3 = 0, int arg4 = 0, int arg5 = 0)
{
    int a;
    asm volatile("int $0x80"
                 : "=a"(a)
                 : "0"(int(num)), "b"(arg1), "c"(arg2), "d"(arg3), "S"(arg4), "D"(arg5));
    return a;
}

int putc(char c)
{
    return do_syscall(Syscall::Putc, c);
}

int exit(int error_code)
{
    return do_syscall(Syscall::Exit, error_code);
}

int fork()
{
    return do_syscall(Syscall::Fork);
}

int read(int fd, void* buf, size_t cnt)
{
    return do_syscall(Syscall::Read, ToSysArg(fd), ToSysArg(buf), ToSysArg(cnt));
}
int write(int fd, void* buf, size_t cnt)
{
    return do_syscall(Syscall::Write, ToSysArg(fd), ToSysArg(buf), ToSysArg(cnt));
}
int lseek(int fd, size_t offset, int whence)
{
    return do_syscall(Syscall::Lseek, ToSysArg(fd), ToSysArg(offset), ToSysArg(whence));
}

int open(const char* filename, int flags, uint16_t mode)
{
    return do_syscall(Syscall::Open, ToSysArg(filename), ToSysArg(flags), ToSysArg(mode));
}

int execve(const char* filename, const char* const* argv, const char* const* envp)
{
    return do_syscall(Syscall::Execve, ToSysArg(filename), ToSysArg(argv), ToSysArg(envp));
}

void* mmap(void* start, uint32_t length, int prot, int flags, int fd, uint32_t offset)
{
    volatile MmapParams params { (uint32_t)start, length, prot, flags, fd, offset };
    return (void*)do_syscall(Syscall::Mmap, ToSysArg(&params));
}

int ioctl(int fd, unsigned long request)
{
    return do_syscall(Syscall::Ioctl, ToSysArg(fd), ToSysArg(request));
}

int socket(int domain, int type, int protocol)
{
    return do_syscall(Syscall::Socket, ToSysArg(domain), ToSysArg(type), ToSysArg(protocol));
}

int bind(int fd, const char* path)
{
    return do_syscall(Syscall::Bind, ToSysArg(fd), ToSysArg(path));
}

int connect(int fd, const char* path)
{
    return do_syscall(Syscall::Connect, ToSysArg(fd), ToSysArg(path));
}

int select(int nfds, fd_set* readfds, fd_set* writefds, fd_set* execfds, void* timeout)
{
    return do_syscall(Syscall::Select, ToSysArg(nfds), ToSysArg(readfds), ToSysArg(writefds), ToSysArg(execfds), ToSysArg(timeout));
}

int getpid()
{
    return do_syscall(Syscall::GetPid);
}