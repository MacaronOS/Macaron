#include "DevFileSystem.hpp"
#include "DevInode.hpp"

namespace Kernel::FileSystem {

using namespace Devices;

void DevFileSystem::init()
{
    auto root = new DevInode(m_nodes_cnt++, *this, nullptr);

    for (auto& device : DeviceManager::the()) {
        String name;

        if (device.major() == 11 && device.minor() == 0 && device.type() == DeviceType::Char) {
            name = "kbd";
        }

        if (device.major() == 13 && device.minor() == 63 && device.type() == DeviceType::Char) {
            name = "mouse";
        }

        if (device.major() == 1 && device.minor() == 1 && device.type() == DeviceType::Block) {
            name = "bga";
        }

        if (device.major() == 5 && device.minor() == 0 && device.type() == DeviceType::Char) {
            name = "ptmx";

            // PTMX also requires a /dev/pts directory.
            root->m_children.push_back(new DevInode(m_nodes_cnt++, *this, nullptr, "pts"));
        }

        if (name.size()) {
            auto node = new DevInode(m_nodes_cnt++, *this, &device, name);
            root->m_children.push_back(node);
        }
    }

    m_root = root;
}

Inode* DevFileSystem::allocate_inode()
{
    return new DevInode(m_nodes_cnt++, *this, nullptr);
}

}