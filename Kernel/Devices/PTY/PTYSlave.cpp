#include "PTYSlave.hpp"
#include "PTYMaster.hpp"
#include <Libkernel/Logger.hpp>
#include <Macaronlib/String.hpp>

namespace Kernel::Devices {

bool PTYSlave::can_read(FileDescription& fd)
{
    return m_buffer.space_to_read_from(fd.offset);
}

void PTYSlave::read(void* buffer, size_t size, FileDescription& fd)
{
    fd.offset += m_buffer.read_from((uint8_t*)buffer, fd.offset, size);
}

bool PTYSlave::can_write(FileDescription&)
{
    return m_master->buffer().space_to_write();
}

void PTYSlave::write(void* buffer, size_t size, FileDescription& fd)
{
    fd.offset += m_master->buffer().write((uint8_t*)buffer, size);
}

}