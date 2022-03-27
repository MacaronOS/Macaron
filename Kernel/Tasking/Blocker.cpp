#include "Blocker.hpp"

namespace Kernel {

bool ReadBlocker::can_unblock() const
{
    return m_fd.file->can_read(m_fd);
}

bool WriteBlocker::can_unblock() const
{
    return m_fd.file->can_write(m_fd);
}

}