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
    offset %= m_buffer_size;
    size_t buffer_index = 0;

    if (offset > m_write_offset) {
        for (; offset < m_buffer_size && buffer_index < size; buffer_index++, offset++) {
            buffer[buffer_index] = m_buffer[offset];
        }
        offset = 0;
    }

    for (; offset < m_write_offset && buffer_index < size; buffer_index++, offset++) {
        buffer[buffer_index] = m_buffer[offset];
    }

    return buffer_index;
}

void LocalSocket::write(uint32_t size, const uint8_t* buffer)
{
    for (uint32_t bytes_written = 0; bytes_written < size; bytes_written++) {
        m_buffer[m_write_offset] = buffer[bytes_written];
        m_write_offset = (m_write_offset + 1) % m_buffer_size;
    }
}

bool LocalSocket::can_read(uint32_t offset) const
{
    return m_write_offset != offset;
}

}