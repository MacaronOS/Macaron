#pragma once

#include "../Device.hpp"
#include <Macaronlib/Ringbuffer.hpp>
#include <Macaronlib/String.hpp>

namespace Kernel::Devices {

class PTYMaster;

class PTYSlave : public Device {
public:
    PTYSlave(size_t number)
        : Device(0, 0, DeviceType::Char)
    {
        m_name = String::From(number);
    }

    const String& name() const { return m_name; }

    virtual bool can_read(FileDescription&) override;
    virtual void read(void* buffer, size_t size, FileDescription& fd) override;
    virtual bool can_write(FileDescription&) override;
    virtual void write(void* buffer, size_t size, FileDescription&) override;

    inline void set_master(PTYMaster* master) { m_master = master; }

    Ringbuffer<1024>& buffer() { return m_buffer; }

private:
    String m_name;
    PTYMaster* m_master;
    Ringbuffer<1024> m_buffer {};
};

}