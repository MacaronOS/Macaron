#pragma once

#include "GICv2Registers.hpp"
#include "InterruptController.hpp"
#include <Devices/Drivers/DeviceDriver.hpp>

namespace Kernel::Devices {

class GICv2 : public Driver,
              public InterruptController {
public:
    REGISTER_DRIVER(GICv2)

    // ^Driver
    virtual String driver_name() override
    {
        return "GICv2";
    }

    virtual String driver_info() override
    {
        return "MacaronOS aarch32 Generic Interrupt Controller v2 driver";
    }

    // ^InterruptController
    virtual void enable_interrupt(uint32_t intno, InterruptType type) override;
    virtual uint32_t acknowledge_interrupt() override;
    virtual void end_of_interrupt(uint32_t intno) override;

protected:
    // ^Driver
    virtual void install() override;

private:
    volatile DistributorRegisters* m_distributor_registers {};
    volatile CPUInterfaceRegisters* m_cpu_interface_registers {};
};

}