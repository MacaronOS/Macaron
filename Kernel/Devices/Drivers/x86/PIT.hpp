#pragma once

#include <Devices/Drivers/Generic/InterruptTimer.hpp>
#include <Hardware/Interrupts/InterruptManager.hpp>

namespace Kernel::Devices {

class PIT : public InterruptTimer,
            public InterruptHandler {
public:
    PIT()
        : InterruptTimer()
        , InterruptHandler(32)
    {
        ACKNOWLEDGE_DRIVER
    }

    // ^Driver
    virtual String driver_name() override
    {
        return "PIT";
    }
    virtual String driver_info() override
    {
        return "MacaronOS x86 Programmable Interrupt Timer driver";
    }

    // ^InterruptHandler
    virtual void handle_interrupt(Trapframe* tf) override;

protected:
    // ^Driver
    virtual void install() override;
};

}