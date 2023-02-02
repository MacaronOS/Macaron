#pragma once

#include "PL011Registers.hpp"

#include <Devices/Drivers/DeviceDriver.hpp>
#include <Devices/Drivers/Generic/Console.hpp>
#include <Devices/Drivers/RegistersMapper.hpp>
#include <Hardware/Interrupts/aarch32/InterruptArchManager.hpp>

namespace Kernel::Devices {

class UartPL011 : public ConsoleDevice<UartPL011>,
                  public InterruptArchHandler {
public:
    UartPL011()
        : InterruptArchHandler(37)
    {
        ACKNOWLEDGE_DRIVER;
    }

    // ^ConsoleDevice
    DriverInstallationResult try_install_console();

    // ^Driver
    virtual String driver_name() override
    {
        return "PL011";
    }
    virtual String driver_info() override
    {
        return "MacaronOS aarch32 PrimeCell UART (PL011) console driver";
    }

    // ^ConsoleDevice
    bool can_receive();
    char receive_byte();
    bool can_send();
    void send_byte(char);

    // ^InterruptHandler
    virtual void handle_interrupt(Trapframe* tf) override;

private:
    RegistersMapper<RegistersDescription<0x1c090000, PL011Registers>> m_registers_mapper;
};

}