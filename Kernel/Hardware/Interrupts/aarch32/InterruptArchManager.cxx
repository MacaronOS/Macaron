#include "InterruptArchManager.hpp"

#include <Hardware/CPU.hpp>
#include <Libkernel/Logger.hpp>

namespace Kernel::Hardware {

void InterruptArchManager::setup_interrupts()
{
    InterruptManager::the().for_each_registered_interrupt([this](InterruptHandler* interrupt) {
        auto arch_interrupt = static_cast<InterruptArchHandler*>(interrupt);
        m_controller->enable_interrupt(arch_interrupt->interrupt_number(), arch_interrupt->type());
    });
}

void InterruptArchManager::handle_interrupt(void* tf)
{
    auto interrupt = m_controller->acknowledge_interrupt();
    m_controller->end_of_interrupt(interrupt);
    Log() << "interrupt " << interrupt << "\n";
    InterruptManager::the().handle_interrupt(interrupt, (Trapframe*)tf);
}

}