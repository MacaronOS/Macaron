#pragma once

#include <Drivers/Base/CharacterDevice.hpp>
#include <Macaronlib/Ringbuffer.hpp>

namespace Kernel {

class PTYSlave;

class PTYMaster : public Drivers::CharacterDevice {
public:
    PTYMaster() = default;

    uint32_t read(uint32_t offset, uint32_t size, void* buffer) override;
    uint32_t write(uint32_t offset, uint32_t size, void* buffer) override;

    inline void set_slave(PTYSlave* slave) { m_slave = slave; }

    Ringbuffer<1024>& buffer() { return m_buffer; }

private:
    PTYSlave* m_slave {};
    Ringbuffer<1024> m_buffer {};
};

};