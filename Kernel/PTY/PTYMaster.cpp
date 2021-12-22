#include "PTYMaster.hpp"
#include "PTYSlave.hpp"

namespace Kernel {

uint32_t PTYMaster::read(uint32_t offset, uint32_t size, void* buffer)
{
    return m_buffer.read((uint8_t*)buffer, size);
}

uint32_t PTYMaster::write(uint32_t offset, uint32_t size, void* buffer)
{
    m_buffer.write((uint8_t*)buffer, size);
    m_slave->buffer().write((uint8_t*)buffer, size);
    return size;
}

}