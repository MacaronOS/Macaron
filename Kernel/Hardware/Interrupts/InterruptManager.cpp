#include "InterruptManager.hpp"

#include <Libkernel/Assert.hpp>
#include <Libkernel/Logger.hpp>

using namespace Kernel;

void InterruptManager::register_interrupt_handler(InterruptHandler* handler)
{
    m_handlers[handler->interrupt_number()] = handler;
}

void InterruptManager::handle_interrupt(uint8_t interrupt, Trapframe* tf)
{
    if (interrupt >= 0 && interrupt < 256 && m_handlers[interrupt]) {
        m_handlers[interrupt]->handle_interrupt(tf);
    } else {
        if (interrupt != 46 && interrupt != 13) { // ignore ata irq for now
            Log() << "Recieved unimplemented interrupt: " << interrupt << "\n";
        }
    }
}