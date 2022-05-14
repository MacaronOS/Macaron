#pragma once

#include <Devices/Device.hpp>
#include <Hardware/Interrupts/InterruptManager.hpp>
#include <Hardware/x86/Port.hpp>

#include <Macaronlib/ABI/Syscalls.hpp>

namespace Kernel::Devices {

class Mouse : public Device, InterruptHandler {
    static constexpr uint8_t data_port = 0x60;
    static constexpr uint8_t command_port = 0x64;

public:
    Mouse();

    bool install() override;
    void handle_interrupt(Trapframe* tf) override;

    virtual bool can_read(FileDescription&) override;
    virtual void read(void* buffer, size_t size, FileDescription& fd) override;

private:
    inline void send_data_to_second_ps2_port(uint8_t data)
    {
        send_command(0xd4);
        write_data(data);
    }

    inline void send_command(uint8_t command)
    {
        if (!wait_before_command()) {
            return;
        }
        outb(command_port, command);
    }

    void write_data(uint8_t data)
    {
        wait_before_data();
        outb(data_port, data);
    }

    uint8_t read_data(void)
    {
        if (!wait_before_data()) {
            return 0;
        }
        return inb(0x60);
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

private:
    uint8_t m_packet[4];
    uint8_t m_packet_ptr {};

    MousePacket* m_packets_buffer {};
    size_t m_packets_buffer_ptr {};

    size_t m_packets_size = { 256 };
};

extern Mouse mouse;

}