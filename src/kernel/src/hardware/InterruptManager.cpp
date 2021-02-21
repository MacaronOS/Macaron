#include "InterruptManager.hpp"
#include <Logger.hpp>
#include <assert.hpp>
#include <monitor.hpp>

template <>
InterruptManager* Singleton<InterruptManager>::s_t = nullptr;
template <>
bool Singleton<InterruptManager>::s_initialized = false;

void InterruptManager::register_interrupt_handler(InterruptHandler* handler)
{
    m_handlers[handler->interrupt_number()] = handler;
}

void InterruptManager::handle_interrupt(trapframe_t* tf)
{
    if (tf->int_no >= 0 && tf->int_no < 256 && m_handlers[tf->int_no]) {
        m_handlers[tf->int_no]->handle_interrupt(tf);
    } else {
        if (tf->int_no != 46) { // ignore ata irq for now
            term_print("\nRecieved unimplemented interrupt, ");
            term_printd(tf->int_no);
            term_print("\n");
        }
    }
}