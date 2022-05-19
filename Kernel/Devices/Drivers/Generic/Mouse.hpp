#pragma once

#include <Devices/Drivers/DeviceDriver.hpp>
#include <Macaronlib/ABI/Syscalls.hpp>
#include <Macaronlib/Ringbuffer.hpp>

namespace Kernel::Devices {

template <typename Implementation>
class Mouse : public CharacterDeviceDriver<Mouse<Implementation>> {
public:
    Mouse()
        : CharacterDeviceDriver<Mouse<Implementation>>(13, 63)
    {
    }

    // ^DeviceDriver
    DriverInstallationResult try_install()
    {
        return static_cast<Implementation*>(this)->try_install();
    }

    // ^File
    bool can_read(FileDescription& fd) override
    {
        return m_buffer.space_to_read_from(fd.offset);
    }
    void read(void* buffer, size_t size, FileDescription& fd) override
    {
        fd.offset += m_buffer.read_from((uint8_t*)buffer, fd.offset, size);
    }

protected:
    void on_mouse_packet(const MousePacket& packet)
    {
        m_buffer.write_force((uint8_t*)(&packet), sizeof(MousePacket));
    }

private:
    Ringbuffer<1024> m_buffer {};
};

}