#include "PIT.hpp"

namespace kernel::drivers {

bool PIT::install()
{
    uint32_t divisor = 1193180 / m_frequency;

    outb(0x43, 0x36);
    outb(0x40, (uint8_t)divisor);
    outb(0x40, (uint8_t)(divisor >> 8));

    return true;
}

void PIT::register_callback(Callback callback)
{
    m_callbacks.push_back(callback);
}

void PIT::handle_interrupt(trapframe_t* tf)
{
    for (size_t i = 0; i < m_callbacks.size(); i++) {
        if (m_ticks_passed % m_callbacks[i].ticks == 0) {
            m_callbacks[i].callback();
        }
    }

    m_ticks_passed++;
}

}