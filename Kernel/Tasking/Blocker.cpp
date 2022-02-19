#include "Blocker.hpp"
#include <Filesystem/Base/FS.hpp>
#include <Filesystem/Base/VNode.hpp>

namespace Kernel {

bool ReadBlocker::can_unblock() const
{
    return m_fd.vnode()->fs()->can_read(*m_fd.vnode(), m_fd.offset());
}

}