#pragma once

#include <algo/Vector.hpp>
#include <drivers/base/CharacterDevice.hpp>
#include <fs/base/VNode.hpp>

namespace kernel::fs::devfs {

using namespace drivers;

class DevFS;
class DevFSNode : public VNode {
    friend class DevFS;

public:
    DevFSNode(FS* fs, uint32_t devnode, CharacterDevice* device=nullptr);

private:
    CharacterDevice* m_device {};
    Vector<DevFSNode*> m_childs {};
};
}