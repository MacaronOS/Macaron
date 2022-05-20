#include "InterruptManager.hpp"

#include <Libkernel/Assert.hpp>
#include <Libkernel/Logger.hpp>

using namespace Kernel;

void InterruptManager::register_interrupt_handler(InterruptHandler* handler)
{
    m_handlers[handler->interrupt_number()] = handler;
}

void InterruptManager::handle_interrupt(Trapframe* tf)
{
    if (tf->int_no >= 0 && tf->int_no < 256 && m_handlers[tf->int_no]) {
        m_handlers[tf->int_no]->handle_interrupt(tf);
    } else {
        if (tf->int_no != 46 && tf->int_no != 13) { // ignore ata irq for now
            Log() << "Recieved unimplemented interrupt: " << tf->int_no << "\n";
        }
    }
}