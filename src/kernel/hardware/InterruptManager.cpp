#include "InterruptManager.hpp"
#include "../assert.hpp"
#include "../monitor.hpp"

InterruptManager* InterruptManager::s_im = nullptr;
bool InterruptManager::initialized = 0;

void InterruptManager::register_interrupt_handler(InterruptHandler* handler)
{
    m_handlers[handler->interrupt_number()] = handler;
}

void InterruptManager::handle_interrupt(trapframe_t* tf)
{
    if (tf->int_no >= 0 && tf->int_no < 256 && m_handlers[tf->int_no]) {
        m_handlers[tf->int_no]->handle_interrupt(tf);
    } else {
        term_print("\nRecieved unimplemented interrupt, ");
        term_printd(tf->int_no);
        term_print("\n");
    }
}