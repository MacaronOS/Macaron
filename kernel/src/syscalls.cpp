#include "syscalls.hpp"
#include "Logger.hpp"
#include "SharedBufferStorage.hpp"
#include "assert.hpp"
#include "fs/vfs/vfs.hpp"
#include "hardware/trapframe.hpp"
#include "monitor.hpp"
#include "multitasking/TaskManager.hpp"

#include <time/TimeManager.hpp>

#include <wisterialib/posix/defines.hpp>
#include <wisterialib/posix/errors.hpp>
#include <wisterialib/posix/shared.hpp>

extern "C" void switch_to_user_mode();

namespace kernel::syscalls {

using namespace Logger;
using namespace memory;
using namespace multitasking;
using namespace fs;
using namespace time;

static int sys_putc(char a)
{
    Log() << "handling putc " << a << "\n";
    term_putc(a);
    return 1;
}

static int sys_exit(int error_code)
{
    TaskManager::the().sys_exit_handler(error_code);
    return 1;
}

static int sys_fork()
{
    return TaskManager::the().sys_fork_handler();
}

static int sys_read(fd_t fd, uint8_t* buf, size_t cnt)
{
    return int(VFS::the().read(fd, buf, cnt));
}

static int sys_write(fd_t fd, uint8_t* buf, size_t cnt)
{
    return int(VFS::the().write(fd, buf, cnt));
}

static int sys_lseek(fd_t fd, size_t offset, int whence) {
    Log() << "handling lseek\n";
    return int(VFS::the().lseek(fd, offset, whence));
}

static int sys_open(const char* filename, int flags, unsigned short mode)
{
    Log() << "handling open\n";
    return int(VFS::the().open(filename, flags, mode));
}

static int sys_execve(const char* filename, const char* const* argv, const char* const* envp)
{
    Log() << "handling excve\n";
    return TaskManager::the().sys_execve_handler(filename, argv, envp);
}

static int sys_mmap(MmapParams* params)
{
    Log() << "handling mmap\n";
    auto cur_process = TaskManager::the().cur_process();

    if (params->flags & MAP_ANONYMOUS) {
        auto mem = cur_process->allocate_space(params->size, Flags::Present | Flags::Write | Flags::User);
        if (!mem) {
            // TODO: implement errno
            return -1;
        }
        return mem.result();
    }

    if (params->fd) {
        uint32_t mem = params->start;
        if (!mem) {
            auto free_space = cur_process->find_free_space(params->size);
            if (!free_space) {
                return -1;
            }
            mem = free_space.result();
        }
        auto error_happened = VFS::the().mmap(params->fd, mem, params->size);
        if (error_happened) {
            return -1;
        }
        return mem;
    }

    return -EBADF;
}

static int sys_write_string(String const* str)
{
    Logger::Log() << "PID" << TaskManager::the().cur_process()->id() << ": " << *str << "\n";
    return 0;
}

static void sys_create_shared_buffer(size_t size, CreateBufferResult* result)
{
    *result = SharedBufferStorage::the().create_buffer(size);
}

static int sys_get_shared_buffer(uint32_t id)
{
    return SharedBufferStorage::the().get_buffer(id);
}

static int sys_ioctl(fd_t fd, uint32_t request)
{
    return int(VFS::the().ioctl(fd, request));
}

static int sys_socket(int domain, int type, int protocol)
{
    return int(VFS::the().socket(domain, type, protocol));
}

static int sys_bind(fd_t fd, const char* path)
{
    return int(VFS::the().bind(fd, path));
}

static int sys_connect(fd_t fd, const char* path)
{
    return int(VFS::the().connect(fd, path));
}

static int sys_can_read(fd_t fd)
{
    return VFS::the().can_read(fd);
}

static int sys_select(int nfds, fd_set* readfds, fd_set* writefds, fd_set* execfds, void* timeout)
{
    return int(VFS::the().select(nfds, readfds, writefds, execfds, timeout));
}

static int sys_getpid()
{
    return TaskManager::the().cur_process()->id();
}

static int sys_clock_gettime(int clock_id, timespec* ts)
{
    auto result =  TimeManager::the().get_time(clock_id);
    if (result.error()) {
        return result.error().posix_error();
    }
    *ts = result.result();
    return 0;
}

SyscallsManager::SyscallsManager()
    : InterruptHandler(0x80)
{
    register_syscall(Syscall::Putc, (uint32_t)sys_putc);
    register_syscall(Syscall::Exit, (uint32_t)sys_exit);
    register_syscall(Syscall::Fork, (uint32_t)sys_fork);
    register_syscall(Syscall::Read, (uint32_t)sys_read);
    register_syscall(Syscall::Write, (uint32_t)sys_write);
    register_syscall(Syscall::Open, (uint32_t)sys_open);
    register_syscall(Syscall::Execve, (uint32_t)sys_execve);
    register_syscall(Syscall::Mmap, (uint32_t)sys_mmap);
    register_syscall(Syscall::Ioctl, (uint32_t)sys_ioctl);
    register_syscall(Syscall::WriteString, (uint32_t)sys_write_string);
    register_syscall(Syscall::CreateSharedBuffer, (uint32_t)sys_create_shared_buffer);
    register_syscall(Syscall::GetSharedBuffer, (uint32_t)sys_get_shared_buffer);
    register_syscall(Syscall::Socket, (uint32_t)sys_socket);
    register_syscall(Syscall::Bind, (uint32_t)sys_bind);
    register_syscall(Syscall::Connect, (uint32_t)sys_connect);
    register_syscall(Syscall::CanRead, (uint32_t)sys_can_read);
    register_syscall(Syscall::Select, (uint32_t)sys_select);
    register_syscall(Syscall::GetPid, (uint32_t)sys_getpid);
    register_syscall(Syscall::Lseek, (uint32_t)sys_lseek);
    register_syscall(Syscall::ClockGettime, (uint32_t)sys_clock_gettime);
}

void SyscallsManager::initialize()
{
    new SyscallsManager();
}

void SyscallsManager::register_syscall(Syscall ss, uint32_t syscall_ptr)
{
    m_syscalls[(uint16_t)ss] = syscall_ptr;
}

void SyscallsManager::handle_interrupt(trapframe_t* regs)
{
    if (regs->eax >= syscall_count || !m_syscalls[regs->eax]) {
        return;
    }
    int ret;
    asm volatile(
        "\
        push %1; \
        push %2; \
        push %3; \
        push %4; \
        push %5; \
        call *%6; \
        pop %%ebx; \
        pop %%ebx; \
        pop %%ebx; \
        pop %%ebx; \
        pop %%ebx; \
        "
        : "=a"(ret)
        : "r"(regs->edi), "r"(regs->esi), "r"(regs->edx), "r"(regs->ecx), "r"(regs->ebx), "r"(m_syscalls[regs->eax]));

    regs->eax = ret;
}

}