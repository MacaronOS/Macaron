#pragma once

#include <Devices/Drivers/DeviceDriver.hpp>
#include <Devices/Drivers/Generic/Mouse.hpp>
#include <Hardware/Interrupts/InterruptManager.hpp>

namespace Kernel::Devices {

class PS2Mouse : public Mouse<PS2Mouse>,
                 public InterruptHandler {
public:
    PS2Mouse()
        : Mouse<PS2Mouse>()
        , InterruptHandler(0x2c)
    {
        ACKNOWLEDGE_DRIVER
    }

    // ^DeviceDriver
    DriverInstallationResult try_install();

    // ^Driver
    virtual String driver_name() override
    {
        return "PS/2-Mouse";
    }
    virtual String driver_info() override
    {
        return "MacaronOS x86 PS/2 mouse driver";
    }

    // ^InterruptHandler
    void handle_interrupt(Trapframe* tf) override;

private:
    uint8_t m_packet[4];
    uint8_t m_packet_ptr {};
};

}