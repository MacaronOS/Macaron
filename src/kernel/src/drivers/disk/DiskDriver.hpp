#pragma once

#include "../base/Driver.hpp"

namespace kernel::drivers {

class DiskDriver : public Driver {
public:
    DiskDriver(DriverEntity driver_entity)
        : Driver(driver_entity)
    {
    }
    virtual bool read(uint32_t, uint8_t, void*) { return false; }
    virtual bool write(uint32_t, uint8_t, void*) { return false; }
};

}