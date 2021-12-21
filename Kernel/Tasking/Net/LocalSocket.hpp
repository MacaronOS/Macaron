#pragma once

#include <Macaronlib/Ringbuffer.hpp>

namespace Kernel::net {

class LocalSocket {
public:
    LocalSocket() = default;

    uint32_t read(uint32_t offset, uint32_t size, uint8_t* buffer);
    void write(uint32_t size, const uint8_t* buffer);
    bool can_read(uint32_t offset) const;

private:
    Ringbuffer<16 * 1024> m_buffer {};
};

}