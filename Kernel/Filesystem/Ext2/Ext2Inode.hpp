#pragma once

#include "Ext2FileSystem.hpp"
#include "Ext2Structs.hpp"
#include <FileSystem/Base/File.hpp>
#include <FileSystem/Base/Inode.hpp>
#include <Macaronlib/String.hpp>

namespace Kernel::FileSystem::Ext2 {

class Ext2Inode : public File, public Inode {
    friend class Ext2FileSystem;

public:
    Ext2Inode(size_t inode, FileSystem& FileSystem)
        : Inode(inode, FileSystem)
        , m_block_size(fs().block_size())
        , m_table_size(m_block_size / sizeof(uint32_t))
    {
    }

    // ^File
    virtual bool can_read(FileDescription&) override;
    virtual void read(void* buffer, size_t size, FileDescription&) override;
    virtual bool can_write(FileDescription&) override;
    virtual void write(void* buffer, size_t size, FileDescription&) override;
    virtual size_t getdents(linux_dirent* dirp, size_t size) override;

    // ^Inode
    virtual void lookup(Dentry& dentry) override;
    virtual void inode_open(FileDescription& fd) override { fd.file = static_cast<File*>(this); }
    virtual Inode* create(const String& name, FileType, FilePermissions) override;

private:
    Ext2FileSystem& fs() { return static_cast<Ext2FileSystem&>(m_fs); }

    size_t read_bytes(void* buffer, size_t size, size_t offset);
    size_t write_bytes(void* buffer, size_t size, size_t offset);

    size_t resolve_local_block(size_t block);
    size_t resolve_local_block_1(size_t indirection_block, size_t block);
    size_t resolve_local_block_2(size_t indirection_block, size_t block);
    size_t resolve_local_block_3(size_t indirection_block, size_t block);

private:
    size_t m_block_size;
    size_t m_table_size; // block indirection table size
    inode_t m_raw_inode {};
};

}