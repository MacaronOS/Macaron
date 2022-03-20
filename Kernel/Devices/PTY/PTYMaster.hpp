#pragma once

#include "../Device.hpp"
#include <Filesystem/DevFS/DevFSNode.hpp>
#include <Macaronlib/Ringbuffer.hpp>

namespace Kernel {

class PTYSlave;

class PTYMaster : public Device {
public:
    PTYMaster()
        : Device(0, 0, DeviceType::Character)
    {
    }

    uint32_t read(uint32_t offset, uint32_t size, void* buffer) override;
    uint32_t write(uint32_t offset, uint32_t size, void* buffer) override;
    bool can_read(uint32_t offset) override { return m_buffer.space_to_read_from(offset); }

    inline void set_slave(PTYSlave* slave) { m_slave = slave; }
    PTYSlave* slave() { return m_slave; }

    Ringbuffer<1024>& buffer() { return m_buffer; }

private:
    PTYSlave* m_slave {};
    Ringbuffer<1024> m_buffer {};
};

};