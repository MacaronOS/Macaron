#include "../Thread.hpp"
#include <Hardware/x86/DescriptorTables/GDT.hpp>

namespace Kernel::Tasking {

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
    trapframe()->eax = (uint32_t)signals().handler(signo);
    trapframe()->eip = (uint32_t)signals().caller_ip();
}

void Thread::return_from_signal_caller()
{
    trapframe()->useresp += sizeof(uint32_t); // Clear signo argument.
    trapframe()->edi = trapframe()->pop();
    trapframe()->esi = trapframe()->pop();
    trapframe()->ebp = trapframe()->pop();
    trapframe()->esp = trapframe()->pop();
    trapframe()->ebx = trapframe()->pop();
    trapframe()->edx = trapframe()->pop();
    trapframe()->ecx = trapframe()->pop();
    trapframe()->eax = trapframe()->pop();
    trapframe()->eip = trapframe()->pop();
    trapframe()->eflags = trapframe()->pop();
    trapframe()->useresp = trapframe()->pop();
}

void Thread::setup_trapframe()
{
    trapframe()->useresp = user_stack_top();
    trapframe()->ebp = user_stack_top();
    trapframe()->eflags = 0x202;
    constexpr uint32_t data = DescriptorTables::GDT::UserDataOffset | DescriptorTables::GDT::RequestRing3;
    constexpr uint32_t code = DescriptorTables::GDT::UserCodeOffset | DescriptorTables::GDT::RequestRing3;
    trapframe()->ds = data;
    trapframe()->es = data;
    trapframe()->fs = data;
    trapframe()->gs = data;
    trapframe()->cs = code;
    trapframe()->ss = data;
}

}