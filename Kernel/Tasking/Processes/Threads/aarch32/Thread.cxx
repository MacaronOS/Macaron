#include "../Thread.hpp"
#include <Hardware/x86/DescriptorTables/GDT.hpp>

namespace Kernel::Tasking {

void Thread::jump_to_signal_caller(int signo)
{
    return;
}

void Thread::return_from_signal_caller()
{
    return;
}

void Thread::setup_trapframe()
{
    return;
}

}