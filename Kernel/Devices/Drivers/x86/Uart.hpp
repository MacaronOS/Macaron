#pragma once

#include <Devices/Drivers/DeviceDriver.hpp>
#include <Devices/Drivers/Generic/Console.hpp>

namespace Kernel::Devices {

class UART : public ConsoleDevice<UART> {
public:
    REGISTER_DRIVER(UART);

    // ^ConsoleDevice
    DriverInstallationResult try_install_console();

    // ^Driver
    virtual String driver_name() override
    {
        return "UART";
    }
    virtual String driver_info() override
    {
        return "MacaronOS x86 UART console driver";
    }

    // ^ConsoleDevice
    bool can_receive();
    char receive_byte();
    bool can_send();
    void send_byte(char);
};

}