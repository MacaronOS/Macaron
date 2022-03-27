#pragma once

#include "DevFileSystem.hpp"

#include <Devices/Device.hpp>
#include <Devices/PTY/PTMX.hpp>
#include <FileSystem/Base/DentryCache.hpp>
#include <FileSystem/Base/Inode.hpp>
#include <Libkernel/Logger.hpp>
#include <Macaronlib/String.hpp>
#include <Macaronlib/Vector.hpp>

namespace Kernel::FileSystem {

class DevInode : public Inode {
    friend class DevFileSystem;
    friend class Devices::PTMX;

public:
    DevInode(size_t inode, FileSystem& fs, Devices::Device* device, const String& name = "")
        : Inode(inode, fs)
        , m_device(device)
        , m_name(name)
    {
    }

    virtual void inode_open(FileDescription& fd) override
    {
        fd.file = m_device;
    }

    virtual void lookup(Dentry& dentry) override
    {
        for (auto child : m_children) {
            if (child->m_name == dentry.name()) {
                dentry.set_inode(child);
                return;
            }
        }
    }

    virtual Inode* create(const String& name, FileType, FilePermissions) override;

private:
    DevFileSystem& fs() { return static_cast<DevFileSystem&>(m_fs); }

private:
    Devices::Device* m_device;
    String m_name;
    Vector<DevInode*> m_children {};
};

}