#include "LocalSocket.hpp"

#include <wisterialib/common.hpp>
#include <wisterialib/memory.hpp>

#include <Logger.hpp>
#include <assert.hpp>

namespace kernel::net {

LocalSocket::LocalSocket()
{
    m_buffer_size = 16 * 1024;
    m_buffer = (uint8_t*)malloc(m_buffer_size);
}

uint32_t LocalSocket::read(uint32_t offset, uint32_t size, uint8_t* buffer)
{
    uint32_t start = offset % m_buffer_size;
    uint32_t left_to_read = size;

    while (true) {
        uint32_t cur_read_bytes = min(m_buffer_size - start, left_to_read);
        memcpy(buffer, &m_buffer[start], cur_read_bytes);
        left_to_read -= cur_read_bytes;
        if (!left_to_read) {
            return start + cur_read_bytes;
        }
        start = 0;
    }
}

void LocalSocket::write(uint32_t size, uint8_t* buffer)
{
    for (uint32_t bytes_written = 0; bytes_written < size; bytes_written++) {
        m_buffer[m_write_offset++] = buffer[bytes_written];
        m_write_offset %= m_buffer_size;
    }
}

bool LocalSocket::can_read(uint32_t offset)
{
    return m_write_offset != offset;
}

}