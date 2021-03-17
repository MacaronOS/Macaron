#pragma once

#include "ext2fs.hpp"

#include <fs/base/VNode.hpp>
#include <fs/base/fs.hpp>
#include <fs/ext2/Ext2.hpp>

namespace kernel::fs::ext2 {

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