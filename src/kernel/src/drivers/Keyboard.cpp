#include "Keyboard.hpp"
#include "../hardware/port.hpp"
#include "../monitor.hpp"

#include <assert.hpp>

namespace kernel::drivers {

bool Keyboard::install()
{
    // wait for a keyboard
    while (inb(command_port) & 0x1) {
        inb(data_port);
    }

    // activate interrupts
    outb(command_port, 0xae);

    // get a state
    outb(command_port, 0x20);
    uint8_t state = (inb(data_port) | 1) & ~0x10;

    // change a state
    outb(command_port, 0x60);
    outb(data_port, state);

    // activate a keyboard
    outb(data_port, 0xf4);
    return true;
}

void Keyboard::handle_interrupt(trapframe_t* tf)
{
    bool released = false;
    uint8_t key_code = inb(data_port);

    if (key_code > 0x80) {
        key_code -= 0x80;
        released = true;
    }

    m_last_keybord_event = { static_cast<Key>(key_code), !released };

    for (size_t i = 0; i < m_callbacks.size(); i++) {
        m_callbacks[i](m_last_keybord_event);
    }
}

void Keyboard::register_callback(void (*callback)(KeyboardEvent))
{
    m_callbacks.push_back(callback);
}

}