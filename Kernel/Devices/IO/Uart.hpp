#pragma once

#include <Devices/Device.hpp>
#include <Hardware/Interrupts/InterruptManager.hpp>
#include <Hardware/x86/Port.hpp>

#include <Macaronlib/ABI/Syscalls.hpp>
#include <Macaronlib/Common.hpp>

namespace Kernel::Devices {

constexpr uint16_t COM1 = 0x3f8;

class Uart : public Device {
public:
    Uart()
        : Device(5, 1, DeviceType::Char)
    {
    }

    bool install() override;
    virtual void read(void* buffer, size_t size, FileDescription& fd) override;
    virtual bool can_write(FileDescription&) override { return true; }
    virtual void write(void* buffer, size_t size, FileDescription&) override;

    void send(char byte);
    char recieve();

private:
    enum class Register : uint16_t {
        Data = COM1 + 0,
        Interrupt,
        Control,
        LineControl,
        ModemControl,
        LineStatus,
        ModemStatus,
        Scratch,
    };

    void write(Uart::Register, char);
    uint8_t read(Uart::Register);

    bool can_send();
    bool can_receive();
};

extern Uart uart;

}