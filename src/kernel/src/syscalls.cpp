#include "syscalls.hpp"
#include "assert.hpp"
#include "fs/vfs.hpp"
#include "hardware/trapframe.hpp"
#include "monitor.hpp"

extern "C" void switch_to_user_mode();

namespace kernel::syscalls {

static int sys_printd(int val)
{
    term_printd(val);

    // blocked
    // stack
    // registers

    // call scheduler

    // Blocker().block();


    

    return 1;
}

SyscallsManager::SyscallsManager()
    : InterruptHandler(0x80)
{
    for (size_t syscall_index = 0; syscall_index < syscall_count; syscall_index++) {
        m_syscalls[syscall_index] = (uint8_t)SyscallSelector::END;
    }

    register_syscall(SyscallSelector::Printd, (uint32_t)sys_printd);
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