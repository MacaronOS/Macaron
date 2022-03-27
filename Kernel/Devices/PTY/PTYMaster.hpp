#pragma once

#include "../Device.hpp"
#include <Macaronlib/Ringbuffer.hpp>
#include <Macaronlib/String.hpp>

namespace Kernel::Devices {

class PTYSlave;

class PTYMaster : public Device {
public:
    PTYMaster()
        : Device(0, 0, DeviceType::Char)
    {
    }

    virtual bool can_read(FileDescription&) override;
    virtual void read(void* buffer, size_t size, FileDescription& fd) override;
    virtual bool can_write(FileDescription&) override;
    virtual void write(void* buffer, size_t size, FileDescription&) override;

    inline void set_slave(PTYSlave* slave) { m_slave = slave; }
    PTYSlave* slave() { return m_slave; }

    Ringbuffer<1024>& buffer() { return m_buffer; }

private:
    PTYSlave* m_slave {};
    Ringbuffer<1024> m_buffer {};
};

};