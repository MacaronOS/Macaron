#include "PIT.hpp"
#include <Hardware/x86/Port.hpp>

namespace Kernel::Devices {

PIT pit;

void PIT::install()
{
    constexpr auto divisor = 1193180 / frequency();

    outb(0x43, 0x36);
    outb(0x40, (uint8_t)divisor);
    outb(0x40, (uint8_t)(divisor >> 8));
}

void PIT::handle_interrupt(Trapframe* tf)
{
    // Pass control to the generic InterruptTimer.
    dispatch_callbacks(tf);
}

}