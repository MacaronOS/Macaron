#pragma once

#include <Macaronlib/Common.hpp>
#include <Macaronlib/String.hpp>

namespace Kernel::Drivers {

class CharacterDevice {
public:
    CharacterDevice(const String& name)
        : m_name(name)
    {
    }

    const String& name() const { return m_name; }

    virtual void open() { }
    virtual uint32_t write(uint32_t offset, uint32_t size, void* buffer) { return 0; }
    virtual uint32_t read(uint32_t offset, uint32_t size, void* buffer) { return 0; }
    virtual bool can_read(uint32_t offset) { return false; }
    virtual bool mmap(uint32_t addr, uint32_t size) { return false; };

    virtual bool ioctl(uint32_t request) { return false; };

protected:
    String m_name;
};

}