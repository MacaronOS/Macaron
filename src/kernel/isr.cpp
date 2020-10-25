#include "isr.hpp"
#include "assert.hpp"
#include "monitor.hpp"
#include "port.hpp"

isr_handler_t interrupt_handlers[256];

void register_interrupt_handler(uint8_t n, isr_handler_t handler)
{
    interrupt_handlers[n] = handler;
}

extern "C" void isr_handler(registers_t regs)
{
    if (interrupt_handlers[regs.int_no] != 0) {
        interrupt_handlers[regs.int_no](&regs);
    } else {
        term_print("recieved interrupt: ");
        term_printd(regs.int_no);
        term_print("\n");
        STOP();
    }
}

extern "C" void irq_handler(registers_t regs)
{
    if (regs.int_no >= 40) {
        outb(0xA0, 0x20);
    }
    outb(0x20, 0x20);

    if (interrupt_handlers[regs.int_no] != 0) {
        interrupt_handlers[regs.int_no](&regs);
    }
}