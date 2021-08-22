#include "DevFSNode.hpp"
#include "DevFS.hpp"

#include <Drivers/Base/CharacterDevice.hpp>
#include <Filesystem/Base/VNode.hpp>

#include <Macaronlib/String.hpp>
#include <Macaronlib/Vector.hpp>

namespace Kernel::FS {

using namespace Drivers;

DevFSNode::DevFSNode(FS* fs, uint32_t devnode, CharacterDevice* device)
    : VNode(fs, devnode)
    , m_device(device)
{
}

}