#include "PS2Mouse.hpp"
#include <Hardware/x86/Port.hpp>
#include <Libkernel/Logger.hpp>

namespace Kernel::Devices {

PS2Mouse ps2mouse;

namespace ps2 {
    constexpr auto data_port = 0x60;
    constexpr auto command_port = 0x64;

    namespace commands {
        constexpr uint8_t enable_auxillary_device = 0xa8;
        constexpr uint8_t read_byte0 = 0x20;
        constexpr uint8_t write_byte0 = 0x60;
        constexpr uint8_t set_defaults = 0xf6;
        constexpr uint8_t enable_mouse_packets = 0xf4;
    }

    struct MousePacket {
        uint8_t left_btn : 1;
        uint8_t right_btn : 1;
        uint8_t middle_btn : 1;
        uint8_t always_1 : 1;
        uint8_t x_sign : 1;
        uint8_t y_sign : 1;
        uint8_t x_overflow : 1;
        uint8_t y_overflow : 1;

        int8_t x_move;
        int8_t y_move;
        int8_t z_move;
    };
}

inline bool wait_before_command()
{
    uint32_t timeout = 100000;
    while (--timeout) {
        if ((inb(0x64) & 2) == 0) {
            return true;
        }
    }
    return false;
}

inline bool wait_before_data()
{
    uint32_t timeout = 100000;
    while (--timeout) {
        if ((inb(0x64) & 1) == 1) {
            return true;
        }
    }
    return false;
}

inline void send_command(uint8_t command)
{
    if (!wait_before_command()) {
        return;
    }
    outb(ps2::command_port, command);
}

void write_data(uint8_t data)
{
    wait_before_data();
    outb(ps2::data_port, data);
}

uint8_t read_data(void)
{
    if (!wait_before_data()) {
        return 0;
    }
    return inb(0x60);
}

inline void send_data_to_second_ps2_port(uint8_t data)
{
    send_command(0xd4);
    write_data(data);
}

DriverInstallationResult PS2Mouse::try_install()
{
    // enable mouse in ps2 controller
    send_command(ps2::commands::enable_auxillary_device);

    // read "byte 0" from internal RAM
    send_command(ps2::commands::read_byte0);

    uint8_t status = read_data() | 2;

    // rewrite "byte 0"
    send_command(ps2::commands::write_byte0);
    write_data(status);

    // restore default settings
    send_data_to_second_ps2_port(ps2::commands::set_defaults);

    // allow packet receiving
    send_data_to_second_ps2_port(ps2::commands::enable_mouse_packets);

    read_data();

    return DriverInstallationResult::Succeeded;
}

void PS2Mouse::handle_interrupt(Trapframe* tf)
{
    uint8_t status = inb(ps2::command_port);
    if (!(status & 0x20)) {
        return;
    }

    auto value = inb(ps2::data_port);
    if (m_packet_ptr == 0 && !(value & (1 << 3))) {
        return;
    }

    m_packet[m_packet_ptr] = value;
    m_packet_ptr = (m_packet_ptr + 1) % 3;

    if (m_packet_ptr == 0) {
        auto packet = reinterpret_cast<ps2::MousePacket*>(m_packet);

        int x_move = packet->x_move;
        int y_move = packet->y_move;

        if (packet->x_overflow) {
            x_move += 255;
        }
        if (packet->y_overflow) {
            y_move += 255;
        }

        MousePacket generic_mouse_packet;

        generic_mouse_packet.left_btn = packet->left_btn;
        generic_mouse_packet.middle_btn = packet->middle_btn;
        generic_mouse_packet.right_btn = packet->right_btn;
        generic_mouse_packet.x_move = packet->x_move;
        generic_mouse_packet.y_move = packet->y_move;

        on_mouse_packet(generic_mouse_packet);
    }
}

}