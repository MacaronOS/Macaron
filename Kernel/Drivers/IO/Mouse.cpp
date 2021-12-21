#include "Mouse.hpp"

#include <Hardware/Port.hpp>
#include <Libkernel/Logger.hpp>

#include <Macaronlib/ABI/Syscalls.hpp>

namespace Kernel::Drivers {

namespace ps2 {
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

Mouse::Mouse()
    : CharacterDeviceDriver(DriverEntity::Mouse, "mouse")
    , InterruptHandler(0x2c)
{
    m_packets_buffer = new MousePacket[m_packets_size];
}

bool Mouse::install()
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

    return true;
}

void Mouse::handle_interrupt(Trapframe* tf)
{
    uint8_t status = inb(command_port);
    if (!(status & 0x20)) {
        return;
    }

    auto value = inb(data_port);
    if (m_packet_ptr == 0 && !(value & (1 << 3))) {
        return;
    }

    m_packet[m_packet_ptr] = value;
    m_packet_ptr = (m_packet_ptr + 1) % 3;

    if (m_packet_ptr == 0) {
        auto packet = (ps2::MousePacket*)(m_packet);

        int x_move = packet->x_move;
        int y_move = packet->y_move;

        if (packet->x_overflow) {
            x_move += 255;
        }
        if (packet->y_overflow) {
            y_move += 255;
        }

        m_packets_buffer[m_packets_buffer_ptr].left_btn = packet->left_btn;
        m_packets_buffer[m_packets_buffer_ptr].middle_btn = packet->middle_btn;
        m_packets_buffer[m_packets_buffer_ptr].right_btn = packet->right_btn;
        m_packets_buffer[m_packets_buffer_ptr].x_move = packet->x_move;
        m_packets_buffer[m_packets_buffer_ptr].y_move = packet->y_move;

        m_packets_buffer_ptr = (m_packets_buffer_ptr + 1) % m_packets_size;
    }
}

uint32_t Mouse::read(uint32_t offset, uint32_t size, void* buffer)
{
    size_t buffer_index = 0;
    size_t packets_index = offset / sizeof(MousePacket) % m_packets_size;

    if (packets_index > m_packets_buffer_ptr) {
        for (; packets_index < m_packets_size && buffer_index < size / sizeof(MousePacket); packets_index++, buffer_index++) {
            reinterpret_cast<MousePacket*>(buffer)[buffer_index] = m_packets_buffer[packets_index];
        }

        packets_index = 0;
    }

    for (; packets_index < m_packets_buffer_ptr && buffer_index < size / sizeof(MousePacket);
         packets_index++, buffer_index++) {

        reinterpret_cast<MousePacket*>(buffer)[buffer_index] = m_packets_buffer[packets_index];
    }

    return buffer_index * sizeof(MousePacket);
}

bool Mouse::can_read(uint32_t offset)
{
    size_t packets_index = offset / sizeof(MousePacket) % m_packets_size;
    return packets_index != m_packets_buffer_ptr;
}

}