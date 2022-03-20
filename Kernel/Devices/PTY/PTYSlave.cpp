#include "PTYSlave.hpp"
#include "PTYMaster.hpp"
#include <Macaronlib/String.hpp>
#include <Libkernel/Logger.hpp>

namespace Kernel {

uint32_t PTYSlave::read(uint32_t offset, uint32_t size, void* buffer)
{
    return m_buffer.read((uint8_t*)buffer, size);
}

uint32_t PTYSlave::write(uint32_t offset, uint32_t size, void* buffer)
{
    return m_master->buffer().write((uint8_t*)buffer, size);
}

}