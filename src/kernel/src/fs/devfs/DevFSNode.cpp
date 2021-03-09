#include "DevFSNode.hpp"
#include "DevFS.hpp"

#include <algo/String.hpp>
#include <algo/Vector.hpp>
#include <drivers/base/CharacterDevice.hpp>
#include <fs/base/VNode.hpp>

namespace kernel::fs::devfs {

using namespace drivers;

DevFSNode::DevFSNode(FS* fs, uint32_t devnode, CharacterDevice* device)
    : VNode(fs, devnode)
    , m_device(device)
{
}


}