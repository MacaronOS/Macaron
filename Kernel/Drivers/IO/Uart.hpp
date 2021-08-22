#pragma once

#include <Drivers/Base/Driver.hpp>

#include <Macaronlib/Common.hpp>

namespace Kernel::Drivers {

constexpr uint16_t COM1 = 0x3f8;

class Uart : public Driver {
public:
    Uart()
        : Driver(DriverEntity::Uart)
    {
    }

    bool install() override;

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

    inline void write(Uart::Register, char);
    inline uint8_t read(Uart::Register);

    inline bool can_send();
    inline bool can_receive();
};
}