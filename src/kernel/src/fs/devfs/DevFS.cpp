#pragma once

#include "DevFS.hpp"
#include "DevFSNode.hpp"

#include <drivers/base/CharacterDevice.hpp>
#include <drivers/DriverManager.hpp>
#include <fs/base/VNode.hpp>
#include <fs/base/fs.hpp>

namespace kernel::fs::devfs {

using namespace drivers;

DevFS::DevFS(VNodeStorage& vnode_storage)
    : m_vnode_storage(vnode_storage)
    , m_root(DevFSNode(this, devnodes++))
{
}

bool DevFS::init()
{
    auto devices = DriverManager::the().get_by_type(Driver::DriverType::CharacterDevice);
    for (size_t device = 0; device < devices.size(); device++) {
        auto node = new DevFSNode(this, devnodes++, reinterpret_cast<CharacterDevice*>(devices[device]));
        m_vnode_storage.push(node);
        m_root.m_childs.push_back(node);
    }
}

VNode* DevFS::finddir(VNode& parent, const String& devname)
{
    auto d_parent = ToDevFSNode(parent);
    for (size_t device = 0; device < d_parent.m_childs.size(); device++) {
        if (d_parent.m_childs[device]->m_device->name() == devname) {
            return d_parent.m_childs[device];
        }
    }
    return nullptr;
}

}