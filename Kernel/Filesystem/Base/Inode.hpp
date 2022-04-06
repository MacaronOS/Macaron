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
    virtual size_t read_bytes(void* buffer, size_t size, size_t offset) { return 0; }
    virtual size_t write_bytes(void* buffer, size_t size, size_t offset) { return 0; }

protected:
    size_t m_inode;
    FileSystem& m_fs;
};

}