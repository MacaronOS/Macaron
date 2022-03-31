#pragma once

#include "File.hpp"
#include <Macaronlib/String.hpp>

namespace Kernel::FileSystem {

class Dentry;
class FileSystem;

class Inode {
public:
    Inode(size_t inode, FileSystem& fs)
        : m_inode(inode)
        , m_fs(fs)
    {
    }

    size_t inode() const { return m_inode; }

    virtual size_t size() const { return 0; }
    virtual void lookup(Dentry&) { }
    virtual void inode_open(FileDescription&) { }
    virtual Inode* create(const String& name, FileType, FilePermissions) { return nullptr; }

protected:
    size_t m_inode;
    FileSystem& m_fs;
};

}