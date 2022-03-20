#include "PIT.hpp"

namespace Kernel::Drivers {

void PIT::initialize()
{
    uint32_t divisor = 1193180 / frequency;

    outb(0x43, 0x36);
    outb(0x40, (uint8_t)divisor);
    outb(0x40, (uint8_t)(divisor >> 8));
}

void PIT::register_tick_reciever(TickReciever* tick_reciever)
{
    m_tick_recievers.push_back(tick_reciever);
}

void PIT::handle_interrupt(Trapframe* tf)
{
    for (size_t i = 0; i < m_tick_recievers.size(); i++) {
        m_tick_recievers[i]->on_tick(tf);
    }
}

}