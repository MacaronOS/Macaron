#include "Thread.hpp"
#include <Libkernel/Logger.hpp>

namespace Kernel::Tasking {

Thread::Thread(Process& process, uintptr_t user_stack)
    : m_process(process)
    , m_user_stack(user_stack)
{
    m_kernel_stack = reinterpret_cast<uintptr_t>(malloc(kernel_stack_size));
    if (user_stack) {
        setup_trapframe();
    }
}

Thread::~Thread()
{
    free(reinterpret_cast<void*>(m_kernel_stack));
}

void Thread::fork_from(Thread& other)
{
    m_user_stack = other.m_user_stack;
    *trapframe() = *other.trapframe();
    trapframe()->eax = 0;
}

}