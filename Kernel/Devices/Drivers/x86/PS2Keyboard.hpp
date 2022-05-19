#pragma once

#include <Devices/Drivers/DeviceDriver.hpp>
#include <Devices/Drivers/Generic/Keyboard.hpp>
#include <Hardware/Interrupts/InterruptManager.hpp>

namespace Kernel::Devices {

class PS2Keyboard : public Keyboard<PS2Keyboard>,
                    public InterruptHandler {
public:
    PS2Keyboard()
        : Keyboard<PS2Keyboard>()
        , InterruptHandler(0x21)
    {
        ACKNOWLEDGE_DRIVER
    }

    // ^DeviceDriver
    DriverInstallationResult try_install();

    // ^Driver
    virtual String driver_name() override
    {
        return "PS/2-Keyboard";
    }
    virtual String driver_info() override
    {
        return "MacaronOS x86 PS/2 keyboard driver";
    }

    // ^InterruptHandler
    void handle_interrupt(Trapframe* tf) override;
};

}