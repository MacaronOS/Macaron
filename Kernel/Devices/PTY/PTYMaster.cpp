#include "PTYMaster.hpp"
#include "PTYSlave.hpp"

#include <Libkernel/Logger.hpp>

namespace Kernel {

uint32_t PTYMaster::read(uint32_t offset, uint32_t size, void* buffer)
{
    return m_buffer.read_from((uint8_t*)buffer, offset, size);
}

uint32_t PTYMaster::write(uint32_t offset, uint32_t size, void* buffer)
{
    m_slave->buffer().write((uint8_t*)buffer, size);
    return 0;
}

}