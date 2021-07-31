#pragma once

#include "Driver.hpp"

#include <Logger.hpp>
#include <wisterialib/String.hpp>
#include <wisterialib/common.hpp>

namespace kernel::drivers {

class CharacterDevice : public Driver {
public:
    CharacterDevice(DriverEntity entity, const String& name)
        : Driver(entity, DriverType::CharacterDevice)
        , m_name(name)
    {
    }

    const String& name() const { return m_name; }

    virtual uint32_t read(uint32_t offset, uint32_t size, void* buffer) { return 0; }
    virtual bool can_read(uint32_t offset) { return false; }
    virtual bool mmap(uint32_t addr, uint32_t size) { return false; };

    virtual bool ioctl(uint32_t request) { return false; };

protected:
    String m_name;
};

}