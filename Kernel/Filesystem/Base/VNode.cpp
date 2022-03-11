#include "VNode.hpp"
#include "FS.hpp"
#include <Macaronlib/String.hpp>

namespace Kernel::FS {

VNode::Mountpoint::Mountpoint(const String& name, VNode* vnode)
    : m_name(name)
    , m_vnode(vnode)
{
}
VNode::Mountpoint::Mountpoint(const Mountpoint& mp)
    : m_name(mp.m_name)
    , m_vnode(mp.m_vnode)
{
}
VNode::Mountpoint::Mountpoint(Mountpoint&& mp)
    : m_name(move(mp.m_name))
    , m_vnode(mp.m_vnode)
{
}
VNode::Mountpoint& VNode::Mountpoint::operator=(const Mountpoint& mp)
{
    m_name = mp.m_name;
    m_vnode = mp.m_vnode;
    return *this;
}
VNode::Mountpoint& VNode::Mountpoint::operator=(Mountpoint&& mp)
{
    m_name = move(mp.m_name);
    m_vnode = mp.m_vnode;
    return *this;
}

VNode::VNode(FS* fs, uint32_t vnode)
    : m_vnode(vnode)
    , m_fs(fs)
{
}

void VNode::read(void* buffer, size_t size, FileDescriptor& fd)
{
    fd.inc_offset(m_fs->read(*this, fd.offset(), size, buffer));
}

void VNode::write(void* buffer, size_t size, FileDescriptor& fd)
{
    fd.inc_offset(m_fs->write(*this, fd.offset(), size, buffer));
}

void VNode::mount(Mountpoint& mountpoint)
{
    m_mountpoints.push_back(mountpoint);
}

void VNode::mount(Mountpoint&& mountpoint)
{
    m_mountpoints.push_back(move(mountpoint));
}

void VNode::lookup(Dentry& dentry)
{
    for (auto& dir : mounted_dirs()) {
        if (dentry.name() == dir.name()) {
            dentry.set_vnode(&dir.vnode());
            return;
        }
    }

    lookup_derived(dentry);
}

VNode* VNodeStorage::find(FS* fs, uint32_t vnode)
{
    for (size_t i = 0; i < vnodes.size(); i++) {
        if (vnodes[i]->vnode() == vnode && vnodes[i]->fs() == fs) {
            return vnodes[i];
        }
    }

    return nullptr;
}

void VNodeStorage::push(VNode* vnode)
{
    vnodes.push_back(vnode);
}

void VNodeStorage::push(VNode& vnode)
{
    vnodes.push_back(&vnode);
}

}