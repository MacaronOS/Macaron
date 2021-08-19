#pragma once

#include "Ext2Structs.hpp"

#include <Filesystem/Base/VNode.hpp>
#include <Filesystem/Base/FS.hpp>
#include <Filesystem/Ext2/Ext2.hpp>

namespace Kernel::FS::EXT2 {

class Ext2Inode : public VNode {
    friend class Ext2;

public:
    Ext2Inode(FS* fs, uint32_t inode, inode_t* inode_struct = nullptr);
    ~Ext2Inode() override;

    // Vnode functions
    uint32_t size() const override;

private:
    inode_t* inode_struct() { return m_inode_struct; };

private:
    inode_t* m_inode_struct;
};

}