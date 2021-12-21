#include "LocalSocket.hpp"

#include <Macaronlib/Common.hpp>
#include <Macaronlib/Memory.hpp>

#include <Libkernel/Logger.hpp>
#include <Libkernel/Assert.hpp>

namespace Kernel::net {

uint32_t LocalSocket::read(uint32_t offset, uint32_t size, uint8_t* buffer)
{
    return m_buffer.read_from(buffer, offset, size);
}

void LocalSocket::write(uint32_t size, const uint8_t* buffer)
{
    return m_buffer.write_force(buffer, size);
}

bool LocalSocket::can_read(uint32_t offset) const
{
    return m_buffer.space_to_read_from(offset);
}

}