#pragma once

#include "PL050Controller.hpp"
#include <Devices/Drivers/DeviceDriver.hpp>
#include <Devices/Drivers/Generic/Mouse.hpp>
#include <Hardware/Interrupts/aarch32/InterruptArchManager.hpp>

namespace Kernel::Devices {

class MousePL050 : public Mouse<MousePL050>,
                   public InterruptArchHandler {
public:
    MousePL050()
        : Mouse<MousePL050>()
        , InterruptArchHandler(45)
    {
        ACKNOWLEDGE_DRIVER;
    }

    // ^DeviceDriver
    DriverInstallationResult try_install();

    // ^Driver
    virtual String driver_name() override
    {
        return "PL050M";
    }

    virtual String driver_info() override
    {
        return "MacaronOS aarch32 PrimeCell PS2 Keyboard/Mouse Interface (PL050) mouse driver";
    }

    // ^InterruptHandler
    void handle_interrupt(Trapframe* tf) override;

private:
    PL050Controller m_controller {};
    uint8_t m_packet[4];
    uint8_t m_packet_ptr {};
};

}