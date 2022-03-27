#pragma once

#include "Inode.hpp"

namespace Kernel::FileSystem {

class FileSystem {
public:
    Inode* root() { return m_root; }

    virtual void init();

    virtual Inode* allocate_inode() { return nullptr; }
    virtual void write_inode(Inode& inode) { }

    virtual size_t block_size() { return 1024; }
    virtual uint32_t allocate_block() { return 0; }
    virtual bool read_block(size_t block, void* buffer) { return false; }
    virtual bool write_block(size_t block, void* buffer) { return false; }

protected:
    Inode* m_root;
};

}