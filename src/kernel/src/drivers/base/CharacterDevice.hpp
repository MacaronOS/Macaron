#pragma once

#include "Driver.hpp"

#include <Logger.hpp>
#include <algo/String.hpp>
#include <types.hpp>

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
    virtual bool mmap(uint32_t addr, uint32_t size) { return false; };

protected:
    String m_name;
};

}