#include "Syscalls.hpp"

#include <Wisterialib/ABI/Syscalls.hpp>
#include <Wisterialib/Common.hpp>
#include <Wisterialib/String.hpp>

#define ToSysArg(arg) ((int)(arg))

static inline int do_syscall(Syscall num, int arg1 = 0, int arg2 = 0, int arg3 = 0, int arg4 = 0, int arg5 = 0)
{
    int a;
    asm volatile("int $0x80"
                 : "=a"(a)
                 : "0"(int(num)), "b"(arg1), "c"(arg2), "d"(arg3), "S"(arg4), "D"(arg5));
    return a;
}

void write_string(const String& str)
{
    do_syscall(Syscall::WriteString, ToSysArg(&str));
}

CreateBufferResult create_shared_buffer(uint32_t size)
{
    CreateBufferResult res;
    do_syscall(Syscall::CreateSharedBuffer, ToSysArg(size), ToSysArg(&res));
    return res;
}

uint32_t get_shared_buffer(uint32_t id)
{
    return do_syscall(Syscall::GetSharedBuffer, ToSysArg(id));
}

bool can_read(int pid)
{
    return do_syscall(Syscall::CanRead, ToSysArg(pid));
}
