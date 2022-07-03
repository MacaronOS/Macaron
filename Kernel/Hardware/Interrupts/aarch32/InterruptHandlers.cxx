#include "InterruptArchManager.hpp"
#include <Hardware/Interrupts/InterruptManager.hpp>
#include <Hardware/aarch32/CPU.hpp>
#include <Memory/VMM/VMM.hpp>

namespace Kernel::Hardware {

using namespace Memory;

extern "C" void irq_handler(Trapframe* regs)
{
    InterruptArchManager::the().handle_interrupt(regs);
}

extern "C" void data_abort_handler(Trapframe* regs)
{
    VMM::the().on_fault(read_data_fault_address(), read_data_fault_status().__bits);
}

extern "C" void prefetch_abort_handler(Trapframe* regs)
{
    VMM::the().on_fault(read_instruction_fault_address(), read_data_fault_status().__bits);
}

}