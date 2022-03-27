#include "PTYMaster.hpp"
#include "PTYSlave.hpp"

#include <Libkernel/Logger.hpp>

namespace Kernel::Devices {

bool PTYMaster::can_read(FileDescription& fd)
{
    return m_buffer.space_to_read_from(fd.offset);
}

void PTYMaster::read(void* buffer, size_t size, FileDescription& fd)
{
    fd.offset += m_buffer.read_from((uint8_t*)buffer, fd.offset, size);
}

bool PTYMaster::can_write(FileDescription&)
{
    return m_slave->buffer().space_to_write();
}

void PTYMaster::write(void* buffer, size_t size, FileDescription& fd)
{
    m_slave->buffer().write((uint8_t*)buffer, size);
}

}