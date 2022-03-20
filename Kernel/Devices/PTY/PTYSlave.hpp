#pragma once

#include "../Device.hpp"
#include <Filesystem/DevFS/DevFSNode.hpp>
#include <Macaronlib/Ringbuffer.hpp>

namespace Kernel {

class PTYMaster;

class PTYSlave : public Device {
public:
    PTYSlave(size_t number)
        : Device(0, 0, DeviceType::Character)
    {
        m_name = String::From(number);
    }

    const String& name() const { return m_name; }

    uint32_t read(uint32_t offset, uint32_t size, void* buffer) override;
    uint32_t write(uint32_t offset, uint32_t size, void* buffer) override;
    bool can_read(uint32_t offset) override { return m_buffer.space_to_read_from(offset); }

    inline void set_master(PTYMaster* master) { m_master = master; }

    Ringbuffer<1024>& buffer() { return m_buffer; }

private:
    String m_name;
    PTYMaster* m_master;
    Ringbuffer<1024> m_buffer {};
};

}