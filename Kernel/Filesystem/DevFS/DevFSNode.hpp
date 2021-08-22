#pragma once

#include <Drivers/Base/CharacterDevice.hpp>
#include <Filesystem/Base/VNode.hpp>

#include <Macaronlib/Vector.hpp>

namespace Kernel::FS {

using namespace Drivers;

class DevFS;
class DevFSNode : public VNode {
    friend class DevFS;

public:
    DevFSNode(FS* fs, uint32_t devnode, CharacterDevice* device = nullptr);

private:
    CharacterDevice* m_device {};
    Vector<DevFSNode*> m_childs {};
};
}