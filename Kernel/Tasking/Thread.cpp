#include "Thread.hpp"

namespace Kernel::Tasking {

using namespace Memory;

void Thread::setup(Thread::Privilege pr)
{
    if (!m_kernel_stack) {
        m_kernel_stack = (uint32_t)malloc(KERNEL_STACK_SIZE);
    }
    m_user_stack = m_process->allocate_space(USER_STACK_SIZE, Flags::Write | Flags::User | Flags::Present, Region::Mapping::Anonimous).result();
    reset_stack();
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

}