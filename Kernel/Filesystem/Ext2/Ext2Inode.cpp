#include "Ext2Inode.hpp"

#include <Filesystem/Base/VNode.hpp>
#include <Macaronlib/Common.hpp>

namespace Kernel::FS::EXT2 {

Ext2Inode::Ext2Inode(FS* fs, uint32_t inode, inode_t* inode_struct)
    : VNode(fs, inode)
    , m_inode_struct(inode_struct)
{
    if (!m_inode_struct) {
        m_inode_struct = new inode_t {};
    }
}

Ext2Inode::~Ext2Inode()
{
    delete m_inode_struct;
}

void Ext2Inode::read(void* buffer, size_t size, FileDescriptor& fd)
{
    if (!can_read(fd)) {
        return;
    }

    size_t read_bytes = 0;
    static uint8_t block_buffer[1024];

    size_t block_size = fs()->block_size();
    size_t block_start = fd.offset() / block_size;
    size_t block_end = min(inode_struct()->size, fd.offset() + size) / block_size;

    // Reading the first block of the block range.
    auto ext2_block = resolve_local_block(block_start);
    fs()->read_block(ext2_block, block_buffer);
    size_t offset_in_start_block = fd.offset() % block_size;
    size_t need_read_from_start_block = min(size, block_size - offset_in_start_block);
    memcpy(buffer, block_buffer + offset_in_start_block, need_read_from_start_block);
    read_bytes += need_read_from_start_block;

    // Reading the middle part of the block range: [start_block + 1 ; enb_block - 1]
    for (size_t block = block_start + 1; block < block_end; block++) {
        ext2_block = resolve_local_block(block);
        fs()->read_block(ext2_block, buffer + read_bytes);
        read_bytes += block_size;
    }

    // Reading the last block of the block range
    if (block_start != block_end) {
        ext2_block = resolve_local_block(block_end);
        fs()->read_block(ext2_block, block_buffer);
        size_t need_read_from_end_block = (fd.offset() + size - need_read_from_start_block) % block_size;
        memcpy(buffer + read_bytes, block_buffer, need_read_from_end_block);
        read_bytes += need_read_from_end_block;
    }

    fd.inc_offset(read_bytes);
}

void Ext2Inode::write(void* buffer, size_t size, FileDescriptor& fd)
{
    size_t written_bytes = 0;
    static uint8_t block_buffer[1024];

    size_t block_size = fs()->block_size();
    size_t block_start = fd.offset() / block_size;
    size_t block_end = min(inode_struct()->size, fd.offset() + size) / block_size;

    // Writing to the first block of the block range.
    auto ext2_block = resolve_local_block(block_start);
    fs()->read_block(ext2_block, block_buffer);
    size_t offset_in_start_block = fd.offset() % block_size;
    size_t need_write_to_start_block = min(size, block_size - offset_in_start_block);
    memcpy(block_buffer + offset_in_start_block, buffer, need_write_to_start_block);
    written_bytes += need_write_to_start_block;

    // Writing to the middle part of the block range: [start_block + 1 ; enb_block - 1]
    for (size_t block = block_start + 1; block < block_end; block++) {
        ext2_block = resolve_local_block(block);
        fs()->write_block(ext2_block, buffer + written_bytes);
        written_bytes += block_size;
    }

    // Reading the last block of the block range
    if (block_start != block_end) {
        ext2_block = resolve_local_block(block_end);
        fs()->read_block(ext2_block, block_buffer);
        size_t need_write_to_end_block = (fd.offset() + size - need_write_to_start_block) % block_size;
        memcpy(block_buffer, buffer + written_bytes, need_write_to_end_block);
        fs()->write_block(ext2_block, block_buffer);
        written_bytes += need_write_to_end_block;
    }
    
    fd.inc_offset(written_bytes);
    if (fd.offset() > inode_struct()->size) {
        inode_struct()->size = fd.offset();
        fs()->write_vnode(*this);
    }
}

bool Ext2Inode::can_read(FileDescriptor& fd)
{
    return fd.offset() < m_inode_struct->size;
}

uint32_t Ext2Inode::size() const
{
    return m_inode_struct->size;
}

void Ext2Inode::lookup_derived(Dentry& dentry)
{
    dentry.set_vnode(static_cast<Ext2*>(fs())->finddir(*this, dentry.name()));
}

size_t Ext2Inode::resolve_local_block(size_t block)
{
    size_t block_size = fs()->block_size();
    size_t table_size = block_size / sizeof(uint32_t);

    if (block < 12) {
        if (!m_inode_struct->direct_block_pointers[block]) {
            m_inode_struct->direct_block_pointers[block] = fs()->allocate_block();
            fs()->write_vnode(*this);
        }
        return m_inode_struct->direct_block_pointers[block];
    }
    block -= 12;

    if (block < table_size) {
        return resolve_local_block_1(m_inode_struct->singly_inderect_block_pointer, block);
    }
    block -= table_size;

    if (block < table_size * table_size) {
        return resolve_local_block_2(m_inode_struct->doubly_inderect_block_pointer, block);
    }
    block -= table_size * table_size;

    return resolve_local_block_3(m_inode_struct->triply_inderect_block_pointer, block);
}

size_t Ext2Inode::resolve_local_block_1(size_t indirection_block, size_t block)
{
    static uint8_t block_buffer[1024];
    fs()->read_block(indirection_block, block_buffer);

    auto table = (uint32_t*)block_buffer;

    if (!table[block]) {
        table[block] = fs()->allocate_block();
        fs()->write_block(indirection_block, block_buffer);
    }

    return table[block];
}

size_t Ext2Inode::resolve_local_block_2(size_t indirection_block, size_t block)
{
    size_t block_size = fs()->block_size();
    size_t table_size = block_size / sizeof(uint32_t);

    static uint8_t block_buffer[1024];
    fs()->read_block(indirection_block, block_buffer);

    auto table = (uint32_t*)block_buffer;
    size_t offset_in_table = block / table_size;

    if (!table[offset_in_table]) {
        table[offset_in_table] = fs()->allocate_block();
        fs()->write_block(indirection_block, block_buffer);
    }

    return resolve_local_block_1(table[offset_in_table], block % table_size);
}

size_t Ext2Inode::resolve_local_block_3(size_t indirection_block, size_t block)
{
    size_t block_size = fs()->block_size();
    size_t table_size = block_size / sizeof(uint32_t);

    static uint8_t block_buffer[1024];
    fs()->read_block(indirection_block, block_buffer);

    auto table = (uint32_t*)block_buffer;
    size_t offset_in_table = block / (table_size * table_size);

    if (!table[offset_in_table]) {
        table[offset_in_table] = fs()->allocate_block();
        fs()->write_block(indirection_block, block_buffer);
    }

    return resolve_local_block_2(table[offset_in_table], block % (table_size * table_size));
}

}