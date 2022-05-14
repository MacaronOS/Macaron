#include "../Syscalls.hpp"

#include <Tasking/Scheduler/Scheduler.hpp>

namespace Kernel::Syscalls {

using namespace Tasking;

void SyscallsManager::handle_interrupt(Trapframe* regs)
{
    if (static_cast<Syscall>(regs->eax) == Syscall::SchedYield) {
        regs->eax = 0;
        Scheduler::the().reschedule();
    }
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