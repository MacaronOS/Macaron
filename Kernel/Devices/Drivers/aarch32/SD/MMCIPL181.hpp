#pragma once

#include "PL181Registers.hpp"
#include "SDCardPL181.hpp"
#include <Devices/Drivers/BusDriver.hpp>

namespace Kernel::Devices {

class MMCIPL181 : public BusDriver {
    friend class SDCardPL181;

public:
    REGISTER_DRIVER(MMCIPL181);

    virtual String driver_name() override
    {
        return "PL181";
    }

    virtual String driver_info()
    {
        return "Macaron OS aarch32 ARM PrimeCell MultiMedia Card Interface (PL181) bus driver";
    }

protected:
    virtual void install() override;

private:
    // SDCardPL181 functions:
    bool set_block_size(uint32_t rca, size_t block_size);
    bool read_block(uint32_t rca, size_t block, size_t block_size, uint32_t* buffer);

    void select_card(uint32_t rca);
    int send_command(uint32_t cmd, uint32_t arg, uint32_t* res = nullptr);

private:
    volatile PL181Registers* m_registers {};
};

}