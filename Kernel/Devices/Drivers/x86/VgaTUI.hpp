#pragma once

#include <Devices/Drivers/DeviceDriver.hpp>
#include <Devices/Drivers/Generic/Console.hpp>

namespace Kernel::Devices {

class VgaTUI : public ConsoleDevice<VgaTUI> {
public:
    REGISTER_DRIVER(VgaTUI);

    // ^DeviceDriver
    DriverInstallationResult try_install();

    // ^Driver
    virtual String driver_name() override
    {
        return "VgaTUI";
    }
    virtual String driver_info() override
    {
        return "MacaronOS x86 VGA Text User Interface console driver";
    }

    // ^ConsoleDevice
    bool can_receive();
    char receive_byte();
    bool can_send();
    void send_byte(char);
};

}