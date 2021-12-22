#pragma once

#include <Drivers/Base/CharacterDevice.hpp>
#include <Macaronlib/Ringbuffer.hpp>

namespace Kernel {

class PTYMaster;

class PTYSlave : public Drivers::CharacterDevice {
public:
    PTYSlave(size_t number);

    uint32_t read(uint32_t offset, uint32_t size, void* buffer) override;
    uint32_t write(uint32_t offset, uint32_t size, void* buffer) override;

    inline void set_master(PTYMaster* master) { m_master = master; }

    Ringbuffer<1024>& buffer() { return m_buffer; }

private:
    PTYMaster* m_master;
    Ringbuffer<1024> m_buffer {};
};

}