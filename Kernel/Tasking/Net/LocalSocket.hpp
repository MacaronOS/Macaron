#pragma once

#include <Macaronlib/Common.hpp>

namespace Kernel::net {

class LocalSocket {
public:
    LocalSocket();

    // reads size bytes from the socket, return read offset
    uint32_t read(uint32_t offset, uint32_t size, uint8_t* buffer);

    void write(uint32_t size, const uint8_t* buffer);

    bool can_read(uint32_t offset) const;

    inline uint32_t write_offset() { return m_write_offset; }

private:
    uint8_t* m_buffer;
    uint32_t m_buffer_size;

    uint32_t m_write_offset { 0 };
};

}