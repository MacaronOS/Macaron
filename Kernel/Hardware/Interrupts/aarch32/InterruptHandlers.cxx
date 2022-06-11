#include "InterruptArchManager.hpp"
#include <Hardware/Interrupts/InterruptManager.hpp>

namespace Kernel::Hardware {

extern "C" void irq_handler(Trapframe* regs)
{
    InterruptArchManager::the().handle_interrupt(regs);
}

}