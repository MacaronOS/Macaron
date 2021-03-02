#include "syscalls.hpp"
#include "Logger.hpp"
#include "assert.hpp"
#include "fs/vfs.hpp"
#include "hardware/trapframe.hpp"
#include "monitor.hpp"
#include "multitasking/TaskManager.hpp"

extern "C" void switch_to_user_mode();

namespace kernel::syscalls {
using namespace Logger;
static int sys_putc(char a)
{
    term_putc(a);
    return 1;
}

static int sys_exit(int error_code)
{
    multitasking::TaskManager::the().sys_exit_handler(error_code);
    return 1;
}

static int sys_fork()
{
    return multitasking::TaskManager::the().sys_fork_handler();
}

static int sys_execve(const char* filename, const char* const* argv, const char* const* envp)
{
    return multitasking::TaskManager::the().sys_execve_handler(filename, argv, envp);
}

SyscallsManager::SyscallsManager()
    : InterruptHandler(0x80)
{
    for (size_t syscall_index = 0; syscall_index < syscall_count; syscall_index++) {
        m_syscalls[syscall_index] = (uint8_t)SyscallSelector::END;
    }

    register_syscall(SyscallSelector::Putc, (uint32_t)sys_putc);
    register_syscall(SyscallSelector::Exit, (uint32_t)sys_exit);
    register_syscall(SyscallSelector::Fork, (uint32_t)sys_fork);
    register_syscall(SyscallSelector::Execve, (uint32_t)sys_execve);
}

void SyscallsManager::initialize()
{
    new SyscallsManager();
}

void SyscallsManager::register_syscall(SyscallSelector ss, uint32_t syscall_ptr)
{
    m_syscalls[(uint8_t)ss] = syscall_ptr;
}

void SyscallsManager::handle_interrupt(trapframe_t* regs)
{
    if (regs->eax == 9) {
        Log() << "execve" << "\n";
    }
    if (regs->eax >= syscall_count || m_syscalls[regs->eax] == (uint8_t)SyscallSelector::END) {
        return;
    }
    int ret;
    asm volatile(
        " \ 
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