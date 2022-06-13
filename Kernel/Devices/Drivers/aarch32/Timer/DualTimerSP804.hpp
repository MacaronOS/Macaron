#pragma once

#include "SP804Registers.hpp"

#include <Devices/Drivers/Generic/InterruptTimer.hpp>
#include <Hardware/Interrupts/aarch32/InterruptArchManager.hpp>

namespace Kernel::Devices {

class DualTimerSP804 : public InterruptTimer,
                       public InterruptArchHandler {
public:
    DualTimerSP804()
        : InterruptTimer()
        , InterruptArchHandler(34, InterruptType::EdgeTriggered)
    {
        ACKNOWLEDGE_DRIVER;
    }

    // ^Driver
    virtual String driver_name() override
    {
        return "SP804";
    }

    virtual String driver_info() override
    {
        return "MacaronOS aarch32 Dual-Timer Module (SP804) driver";
    }

    // ^InterruptHandler
    virtual void handle_interrupt(Trapframe* tf) override;

protected:
    // ^Driver
    virtual void install() override;

private:
    volatile SP804Registers* m_registers {};
};

}