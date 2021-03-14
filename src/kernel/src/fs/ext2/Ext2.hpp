#pragma once

#include "ext2fs.hpp"
#include "Ext2Inode.hpp"

#include <types.hpp>

#include <fs/base/fs.hpp>
#include <fs/base/VNode.hpp>
#include <algo/Bitmap.hpp>
#include <algo/String.hpp>
#include <algo/Vector.hpp>
#include <drivers/disk/Ata.hpp>
#include <drivers/disk/DiskDriver.hpp>

namespace kernel::fs::ext2 {

struct inode_cache_t {
    uint32_t inode;
    inode_t inode_struct;
};

class Ext2Inode;

class Ext2 : public FS {
public:
    Ext2(drivers::DiskDriver&, VNodeStorage& file_storage);
    ~Ext2();

    bool init();

    VNode& root() override { return *m_root; }

    // file system api functions
    uint32_t read(VNode& file, uint32_t offset, uint32_t size, void* buffer) override;
    uint32_t write(VNode& file, uint32_t offset, uint32_t size, void* buffer) override;
    uint32_t truncate(VNode& file, uint32_t size) override;

    VNode* finddir(VNode& directory, const String& filename) override;
    Vector<String> listdir(VNode& directory) override;

    VNode* create(VNode& directory, const String& name, FileType type, file_permissions_t perms) override;
    bool erase(VNode& directory, const VNode& file) override;

private:
    drivers::DiskDriver& m_disk_driver;
    VNodeStorage& m_file_storage;

    // root
    VNode* m_root;

    // file system params
    ext2_superblock_t m_superblock;
    block_group_descriptor_t* m_bgd_table {};
    uint32_t m_block_size; // size of a block in bytes
    uint32_t m_bgd_table_size;

    // preallocated buffers reduce heap allocation calls
    char* m_block_buffer {};
    char* m_table_buffer_1 {};
    char* m_table_buffer_2 {};
    char* m_table_buffer_3 {};

    static Ext2Inode& ToExt2Inode(VNode& file) { return reinterpret_cast<Ext2Inode&>(file); }
    static Ext2Inode* ToExt2Inode(VNode* file) { return reinterpret_cast<Ext2Inode*>(file); }

    // driver related
    bool read_blocks(uint32_t block, uint32_t block_size, void* mem);
    bool read_block(uint32_t block, void* mem);
    bool write_blocks(uint32_t block, uint32_t block_size, void* mem);
    bool write_block(uint32_t block, void* mem);

    // inode helpers
    inode_t get_inode_structure(uint32_t inode);
    bool save_inode_structure(Ext2Inode& file);
    bool save_inode_structure(Ext2Inode* file);
    uint32_t resolve_inode_local_block(Ext2Inode& file, uint32_t block, bool need_create = false);
    uint32_t read_inode_content(Ext2Inode& file, uint32_t offset, uint32_t size, void* mem);
    uint32_t write_inode_content(Ext2Inode& file, uint32_t offset, uint32_t size, void* mem);

    uint32_t occypy_inode(uint32_t preferd_block_group = 0);
    bool free_inode(uint32_t inode);

    // block helpers
    uint32_t occypy_block(uint32_t preferd_block_group = 0, bool fill_zeroes = false);
    bool free_block(uint32_t block);
};
}