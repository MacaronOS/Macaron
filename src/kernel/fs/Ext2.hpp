#pragma once

#include "fs.hpp"
#include "ext2fs.hpp"

#include "../algo/Bitmap.hpp"
#include "../algo/String.hpp"
#include "../algo/Vector.hpp"
#include "../drivers/disk/Ata.hpp"
#include "../drivers/disk/DiskDriver.hpp"
#include "../types.hpp"


namespace kernel::fs::ext2 {
using algorithms::Bitmap;
using algorithms::String;
using algorithms::Vector;

struct inode_cache_t {
    uint32_t inode;
    inode_t inode_struct;
};

class Ext2 : public FS {
public:
    Ext2(drivers::DiskDriver&, FileStorage& file_storage);
    ~Ext2();

    bool init();

    File& root() override { return *m_root; }

    // file system api functions
    uint32_t read(File& file, uint32_t offset, uint32_t size, void* buffer) override;
    uint32_t write(File& file, uint32_t offset, uint32_t size, void* buffer) override;

    File* finddir(File& directory, const String& filename) override;
    Vector<String> listdir(File& directory) override;

    File& create(File& directory, const String& name, FileType type, file_permissions_t perms) override;
    bool erase(File& directory, const File& file) override;

private:
    drivers::DiskDriver& m_disk_driver;
    FileStorage& m_file_storage;

    // root
    File* m_root;

    // file system params
    ext2_superblock_t m_superblock;
    block_group_descriptor_t* m_bgd_table {};
    uint32_t m_block_size;
    uint32_t m_bgd_table_size;

    // buffers
    char* m_block_buffer {};
    char* m_table_buffer_1 {};
    char* m_table_buffer_2 {};
    char* m_table_buffer_3 {};

    // driver based
    bool read_blocks(uint32_t block, uint32_t block_size, void* mem);
    bool read_block(uint32_t block, void* mem);
    bool write_blocks(uint32_t block, uint32_t block_size, void* mem);
    bool write_block(uint32_t block, void* mem);

    // inode helpers
    inode_t get_inode_structure(uint32_t inode);
    bool save_inode_structure(File& file);
    uint32_t resolve_inode_local_block(File& file, uint32_t block, bool need_create = false);
    uint32_t read_inode_content(File& file, uint32_t offset, uint32_t size, void* mem);
    uint32_t write_inode_content(File& file, uint32_t offset, uint32_t size, void* mem);
    uint32_t occypy_inode(uint32_t preferd_block_group = 0);
    bool free_inode(uint32_t inode);

    // block helpers
    uint32_t occypy_block(uint32_t preferd_block_group = 0, bool fill_zeroes = false);
};
}