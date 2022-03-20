#include "DevFS.hpp"
#include "DevFSNode.hpp"

#include <Devices/DeviceManager.hpp>
#include <Devices/PTY/PTMX.hpp>
#include <Drivers/Base/CharacterDevice.hpp>
#include <Drivers/Base/CharacterDeviceDriver.hpp>
#include <Drivers/DriverManager.hpp>
#include <Filesystem/Base/FS.hpp>
#include <Filesystem/Base/VNode.hpp>
#include <Libkernel/Logger.hpp>
#include <Memory/vmm.hpp>

namespace Kernel::FS {

using namespace Drivers;
using namespace Memory;

DevFS::DevFS(VNodeStorage& vnode_storage)
    : m_vnode_storage(vnode_storage)
    , m_root(DevFSNode(this, devnodes++))
{
}

bool DevFS::init()
{
    for (auto& device : DeviceManager::the()) {
        String name;

        if (device.major() == 11 && device.minor() == 0 && device.type() == DeviceType::Character) {
            name = "kbd";
        }

        if (device.major() == 13 && device.minor() == 63 && device.type() == DeviceType::Character) {
            name = "mouse";
        }

        if (device.major() == 1 && device.minor() == 1 && device.type() == DeviceType::Block) {
            name = "bga";
        }

        if (name.size()) {
            auto node = new DevFSNode(this, devnodes++, &device, name);
            m_vnode_storage.push(node);
            m_root.m_childs.push_back(node);
        }
    }

    auto pts_directory = static_cast<DevFSNode*>(mkdir(root(), "pts"));
    auto ptmx = new PTMX(*pts_directory);
    DeviceManager::the().register_device(ptmx);
    create_device_node_inside_directory(root(), ptmx, "ptmx");
    return true;
}

VNode* DevFS::finddir(VNode& parent, const String& devname)
{
    auto d_parent = ToDevFSNode(parent);

    for (auto child : d_parent.m_childs) {
        if (child->m_virtual_name == devname) {
            return child;
        }
    }

    return nullptr;
}

uint32_t DevFS::read(VNode& file, uint32_t offset, uint32_t size, void* buffer)
{
    return ToDevFSNode(file).m_device->read(offset, size, buffer);
}

uint32_t DevFS::write(VNode& file, uint32_t offset, uint32_t size, void* buffer)
{
    return ToDevFSNode(file).m_device->write(offset, size, buffer);
}

bool DevFS::mmap(VNode& file, uint32_t addr, uint32_t size)
{
    return ToDevFSNode(file).m_device->mmap(addr, size);
}

bool DevFS::ioctl(VNode& file, uint32_t request)
{
    return ToDevFSNode(file).m_device->ioctl(request);
}

bool DevFS::can_read(VNode& vnode, uint32_t offset)
{
    return ToDevFSNode(vnode).m_device->can_read(offset);
}

VNode* DevFS::mkdir(VNode& directory, const String& name)
{
    auto node = new DevFSNode(this, devnodes++, nullptr, name);
    m_vnode_storage.push(node);
    ToDevFSNode(directory).m_childs.push_back(node);
    return node;
}

void DevFS::open(VNode& file, FileDescriptor& fd)
{
    ToDevFSNode(file).m_device->open(fd, file);
}

VNode* DevFS::create_device_node_inside_directory(VNode& directory, Device* deivce, const String& name)
{
    auto node = new DevFSNode(this, devnodes++, deivce, name);
    m_vnode_storage.push(node);
    ToDevFSNode(directory).m_childs.push_back(node);
    return node;
}

VNode* DevFS::create_anonim_device_node(Device* device)
{
    auto node = new DevFSNode(this, devnodes++, device);
    m_vnode_storage.push(node);
    return node;
}

}