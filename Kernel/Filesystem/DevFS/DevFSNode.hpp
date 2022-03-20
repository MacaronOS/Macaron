#pragma once

#include <Devices/Device.hpp>
#include <Drivers/Base/CharacterDevice.hpp>
#include <Filesystem/Base/VNode.hpp>

#include <Macaronlib/Vector.hpp>

namespace Kernel::FS {

using namespace Drivers;
using namespace Devices;

class DevFS;
class DevFSNode : public VNode {
    friend class DevFS;

public:
    DevFSNode(FS* fs, uint32_t devnode, Device* device = nullptr, const String& virtual_name = "");
    Device* device() { return m_device; }

    virtual void lookup_derived(Dentry& dentry) override;

private:
    Device* m_device {};
    String m_virtual_name {};
    Vector<DevFSNode*> m_childs {};
};
}