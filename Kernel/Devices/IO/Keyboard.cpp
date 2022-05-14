#include "Keyboard.hpp"

#include <Hardware/x86/Port.hpp>
#include <Libkernel/Assert.hpp>
#include <Libkernel/Logger.hpp>

namespace Kernel::Devices {

Keyboard keyboard;

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

void Keyboard::handle_interrupt(Trapframe* tf)
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

    m_buffer.write_force((uint8_t*)(&m_last_keybord_event), sizeof(m_last_keybord_event));
}

bool Keyboard::can_read(FileDescription& fd)
{
    return m_buffer.space_to_read_from(fd.offset);
}

void Keyboard::read(void* buffer, size_t size, FileDescription& fd)
{
    fd.offset += m_buffer.read_from((uint8_t*)buffer, fd.offset, size);
}

}