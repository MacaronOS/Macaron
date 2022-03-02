#include "DevFSNode.hpp"
#include "DevFS.hpp"

#include <Drivers/Base/CharacterDevice.hpp>
#include <Filesystem/Base/VNode.hpp>

#include <Macaronlib/String.hpp>
#include <Macaronlib/Vector.hpp>

namespace Kernel::FS {

using namespace Drivers;

DevFSNode::DevFSNode(FS* fs, uint32_t devnode, CharacterDevice* device, const String& virtual_name)
    : VNode(fs, devnode)
    , m_device(device)
    , m_virtual_name(virtual_name)
{
}

void DevFSNode::lookup_derived(Dentry& dentry)
{
    for (auto child : m_childs) {
        if (child->device()) {
            if (child->device()->name() == dentry.name()) {
                dentry.set_vnode(child);
                return;
            }
        } else {
            if (child->m_virtual_name == dentry.name()) {
                dentry.set_vnode(child);
                return;
            }
        }
    }
}

}