#pragma once

#include "Ext2Structs.hpp"
#include <Devices/Device.hpp>
#include <FileSystem/Base/FileSystem.hpp>

namespace Kernel::FileSystem::Ext2 {

using namespace Devices;

class Ext2Inode;

class Ext2FileSystem : public FileSystem {
public:
    Ext2FileSystem(BlockDevice& block_device)
        : m_block_device(block_device)
    {
    }

    virtual void init() override;
    virtual Inode* allocate_inode() override;
    virtual void write_inode(Inode& inode) override;
    virtual size_t block_size() override;
    virtual uint32_t allocate_block() override;
    virtual bool read_block(size_t block, void* buffer) override;
    virtual bool write_block(size_t block, void* buffer) override;

    Ext2Inode* get_inode(size_t inode);
    char* block_buffer() { return m_block_buffer.data(); }

private:
    uint32_t occypy_inode(uint32_t preferd_block_group = 0);
    char* own_block_buffer() { return m_own_block_buffer.data(); }

private:
    BlockDevice& m_block_device;
    ext2_superblock_t m_superblock;
    uint32_t m_block_size;
    uint32_t m_bgd_table_size;
    block_group_descriptor_t* m_bgd_table {};
    Vector<char> m_block_buffer {};
    Vector<char> m_own_block_buffer {};
};

}