#pragma once

#include "Driver.hpp"

#include <types.hpp>

#include <algo/String.hpp>

namespace kernel::drivers {

class CharacterDevice {
public:
    CharacterDevice(const String& name)
        : m_name(name)
    {
    }
    
    virtual uint32_t read(uint32_t offset, uint32_t size, void* buffer) { return 0; }
    virtual void mmap() {};

    const String& name() const { return m_name; }

private:
    String m_name;
};
}