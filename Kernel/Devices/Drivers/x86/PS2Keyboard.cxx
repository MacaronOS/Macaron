#include "PS2Keyboard.hpp"
#include <Hardware/x86/Port.hpp>

namespace Kernel::Devices {

PS2Keyboard ps2keyboard;

constexpr auto data_port = 0x60;
constexpr auto command_port = 0x64;

DriverInstallationResult PS2Keyboard::try_install()
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

    return DriverInstallationResult::Succeeded;
}

void PS2Keyboard::handle_interrupt(Trapframe* tf)
{
    bool pressed = true;
    uint8_t key_code = inb(data_port);
    if (key_code > 0x80) {
        key_code -= 0x80;
        pressed = false;
    }

    KeyboardPacket packet;
    packet.key = static_cast<Key>(key_code);
    packet.pressed = pressed;

    // Dispatch a packet to the generic implementation.
    on_keyboard_packet(packet);
}

}