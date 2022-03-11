#pragma once

#include "Ext2Structs.hpp"

#include <Filesystem/Base/FS.hpp>
#include <Filesystem/Base/VNode.hpp>
#include <Filesystem/Ext2/Ext2.hpp>

namespace Kernel::FS::EXT2 {

class Ext2Inode : public VNode {
    friend class Ext2;

public:
    Ext2Inode(FS* fs, uint32_t inode, inode_t* inode_struct = nullptr);
    ~Ext2Inode() override;

    virtual void read(void* buffer, size_t size, FileDescriptor&) override;
    virtual void write(void* buffer, size_t size, FileDescriptor&) override;
    virtual bool can_read(FileDescriptor&) override;

    uint32_t size() const override;

private:
    inode_t* inode_struct() { return m_inode_struct; };

    virtual void lookup_derived(Dentry& dentry) override;

    size_t resolve_local_block(size_t block);
    size_t resolve_local_block_1(size_t indirection_block, size_t block);
    size_t resolve_local_block_2(size_t indirection_block, size_t block);
    size_t resolve_local_block_3(size_t indirection_block, size_t block);

private:
    inode_t* m_inode_struct;
};

}