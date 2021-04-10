#include "syscalls.hpp"
#include "Logger.hpp"
#include "SharedBufferStorage.hpp"
#include "assert.hpp"
#include "fs/vfs/vfs.hpp"
#include "hardware/trapframe.hpp"
#include "monitor.hpp"
#include "multitasking/TaskManager.hpp"

#include <wisterialib/posix/defines.hpp>
#include <wisterialib/posix/errors.hpp>
#include <wisterialib/posix/shared.hpp>

extern "C" void switch_to_user_mode();

namespace kernel::syscalls {

using namespace Logger;
using namespace memory;
using namespace multitasking;
using namespace fs;

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

static int sys_open(const char* filename, int flags, unsigned short mode)
{
    Log() << "handling open\n";

    auto res = VFS::the().open(filename, flags, mode);
    if (!res) {
        return res.error().posix_error();
    }
    return res.result();
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

SyscallsManager::SyscallsManager()
    : InterruptHandler(0x80)
{
    for (uint32_t& m_syscall : m_syscalls) {
        m_syscall = (uint8_t)Syscall::END;
    }

    register_syscall(Syscall::Putc, (uint32_t)sys_putc);
    register_syscall(Syscall::Exit, (uint32_t)sys_exit);
    register_syscall(Syscall::Fork, (uint32_t)sys_fork);
    register_syscall(Syscall::Open, (uint32_t)sys_open);
    register_syscall(Syscall::Execve, (uint32_t)sys_execve);
    register_syscall(Syscall::Mmap, (uint32_t)sys_mmap);
    register_syscall(Syscall::WriteString, (uint32_t)sys_write_string);
    register_syscall(Syscall::CreateSharedBuffer, (uint32_t)sys_create_shared_buffer);
    register_syscall(Syscall::GetSharedBuffer, (uint32_t)sys_get_shared_buffer);
}

void SyscallsManager::initialize()
{
    new SyscallsManager();
}

void SyscallsManager::register_syscall(Syscall ss, uint32_t syscall_ptr)
{
    m_syscalls[(uint8_t)ss] = syscall_ptr;
}

void SyscallsManager::handle_interrupt(trapframe_t* regs)
{
    if (regs->eax >= syscall_count || m_syscalls[regs->eax] == (uint8_t)Syscall::END) {
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