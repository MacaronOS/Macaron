#pragma once

#include "PL050Controller.hpp"
#include <Devices/Drivers/DeviceDriver.hpp>
#include <Devices/Drivers/Generic/Keyboard.hpp>
#include <Hardware/Interrupts/aarch32/InterruptArchManager.hpp>

namespace Kernel::Devices {

class KeyboardPL050 : public Keyboard<KeyboardPL050>,
                      public InterruptArchHandler {
public:
    KeyboardPL050()
        : Keyboard<KeyboardPL050>()
        , InterruptArchHandler(44)
    {
        ACKNOWLEDGE_DRIVER;
    }

    // ^DeviceDriver
    DriverInstallationResult try_install();

    // ^Driver
    virtual String driver_name() override
    {
        return "PL050K";
    }

    virtual String driver_info() override
    {
        return "MacaronOS x86 PS/2 keyboard driver";
    }

    // ^InterruptHandler
    void handle_interrupt(Trapframe* tf) override;

private:
    PL050Controller m_controller {};
};

}