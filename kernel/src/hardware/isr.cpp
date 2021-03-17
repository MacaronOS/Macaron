#include "InterruptManager.hpp"

#include <assert.hpp>
#include <hardware/port.hpp>
#include <monitor.hpp>

extern "C" void isr_handler(trapframe_t* regs)
{
    InterruptManager::the().handle_interrupt(regs);
}

extern "C" void irq_handler(trapframe_t* regs)
{
    if (regs->int_no >= 40) {
        outb(0xA0, 0x20);
    }
    outb(0x20, 0x20);

    InterruptManager::the().handle_interrupt(regs);
}