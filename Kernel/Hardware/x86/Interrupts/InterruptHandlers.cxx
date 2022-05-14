#include <Hardware/Interrupts/InterruptManager.hpp>
#include <Hardware/x86/Port.hpp>

extern "C" void isr_handler(Trapframe* regs)
{
    InterruptManager::the().handle_interrupt(regs);
}

extern "C" void irq_handler(Trapframe* regs)
{
    if (regs->int_no >= 40) {
        outb(0xA0, 0x20);
    }
    outb(0x20, 0x20);

    InterruptManager::the().handle_interrupt(regs);
}