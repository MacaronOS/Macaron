#include "Thread.hpp"
#include <Memory/vmm.hpp>
#include <Tasking/Process.hpp>

namespace Kernel::Tasking {

using namespace Memory;

void Thread::setup(Thread::Privilege pr)
{
    if (!m_kernel_stack) {
        m_kernel_stack = (uint32_t)malloc(KERNEL_STACK_SIZE);
    }

    reset_flags();
    set_privilege(pr);
}

Thread* Thread::TieNewTo(Process* proc)
{
    auto thread = new Thread(proc);
    proc->add_thread(thread);
    proc->TS().push_back(thread);
    return thread;
}

void Thread::jump_to_signal_caller(int signo)
{
    trapframe()->push(trapframe()->useresp);
    trapframe()->push(trapframe()->eflags);
    trapframe()->push(trapframe()->eip);
    trapframe()->push(trapframe()->eax);
    trapframe()->push(trapframe()->ecx);
    trapframe()->push(trapframe()->edx);
    trapframe()->push(trapframe()->ebx);
    trapframe()->push(trapframe()->esp);
    trapframe()->push(trapframe()->ebp);
    trapframe()->push(trapframe()->esi);
    trapframe()->push(trapframe()->edi);
    trapframe()->push(signo);
    trapframe()->eax = (uint32_t)signal_handler(signo);
    trapframe()->eip = Scheduler::the().m_signal_handler_ip;
}

}