#include "Ext2Inode.hpp"

#include <FileSystem/Base/DentryCache.hpp>
#include <Libkernel/Logger.hpp>
#include <Macaronlib/Memory.hpp>

namespace Kernel::FileSystem::Ext2 {

bool Ext2Inode::can_read(FileDescription& fd)
{
    return fd.offset < m_raw_inode.size;
}

void Ext2Inode::read(void* buffer, size_t size, FileDescription& fd)
{
    fd.offset += read_bytes(buffer, size, fd.offset);
}

bool Ext2Inode::can_write(FileDescription& fd)
{
    size_t max_blocks_for_inode = 0;
    max_blocks_for_inode += 12;
    max_blocks_for_inode += m_table_size;
    max_blocks_for_inode += m_table_size * m_table_size;
    max_blocks_for_inode += m_table_size * m_table_size * m_table_size;

    return fd.offset < max_blocks_for_inode * m_block_size;
}

void Ext2Inode::write(void* buffer, size_t size, FileDescription& fd)
{
    fd.offset += write_bytes(buffer, size, fd.offset);
}

size_t Ext2Inode::getdents(linux_dirent* dirp, size_t size)
{
    auto directory = (uint8_t*)malloc(m_raw_inode.size);
    size_t bytes = read_bytes(directory, m_raw_inode.size, 0);

    size_t ext2_dir_entry_offset = 0;
    size_t linux_dir_entry_offset = 0;
    linux_dirent* last_linux_dir_entry = nullptr;

    while (ext2_dir_entry_offset < bytes && linux_dir_entry_offset < size) {
        auto ext2_dir_entry = (dir_entry_t*)(directory + ext2_dir_entry_offset);
        auto linux_dir_entry = (linux_dirent*)((size_t)dirp + linux_dir_entry_offset);

        ext2_dir_entry_offset += ext2_dir_entry->size;

        auto linux_dir_entry_size = 2 * sizeof(uint32_t) + sizeof(uint16_t) + ext2_dir_entry->name_len_low + 1;
        if (linux_dir_entry_offset + linux_dir_entry_size > size) {
            break;
        }

        static char name[256];
        memcpy(name, ext2_dir_entry->name_characters, ext2_dir_entry->name_len_low);
        name[ext2_dir_entry->name_len_low] = '\0';

        // Skip Ext2 irrelevant entries
        if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
            continue;
        }

        memcpy(linux_dir_entry->d_name, name, ext2_dir_entry->name_len_low + 1);

        linux_dir_entry->d_ino = ext2_dir_entry->inode;
        linux_dir_entry->d_reclen = linux_dir_entry_size;

        linux_dir_entry_offset += linux_dir_entry_size;

        if (last_linux_dir_entry) {
            last_linux_dir_entry->d_off = (size_t)linux_dir_entry - (size_t)last_linux_dir_entry;
        }

        last_linux_dir_entry = linux_dir_entry;
    }

    free(directory);

    return linux_dir_entry_offset;
}

void Ext2Inode::lookup(Dentry& dentry)
{
    auto directory = (uint8_t*)malloc(m_raw_inode.size);
    size_t bytes = read_bytes(directory, m_raw_inode.size, 0);

    size_t dir_entry_offset = 0;

    while (dir_entry_offset < bytes) {
        auto dir_entry = (dir_entry_t*)(directory + dir_entry_offset);

        char name[255];
        memcpy(name, dir_entry->name_characters, dir_entry->name_len_low);
        name[dir_entry->name_len_low] = '\0';

        if (dentry.name() == name) {
            dentry.set_inode(fs().get_inode(dir_entry->inode));
            break;
        }

        dir_entry_offset += dir_entry->size;
    }

    free(directory);
}

Inode* Ext2Inode::create(const String& name, FileType type, FilePermissions permissions)
{
    auto ext2_inode = static_cast<Ext2Inode*>(fs().allocate_inode());

    ext2_inode->m_raw_inode.type_and_permissions = static_cast<uint16_t>(type) | permissions;
    ext2_inode->m_raw_inode.user_id = 1;
    ext2_inode->m_raw_inode.size = 0;
    ext2_inode->m_raw_inode.c_time = 116;

    fs().write_inode(*ext2_inode);

    static uint8_t dir_entry_buffer[sizeof(dir_entry_t) + 255];
    auto dir_entry = (dir_entry_t*)dir_entry_buffer;

    dir_entry->inode = ext2_inode->inode();
    dir_entry->size = sizeof(dir_entry_t) + name.size();
    dir_entry->name_len_low = name.size();

    for (int i = 0; i < name.size(); i++) {
        dir_entry->name_characters[i] = name[i];
    }

    write_bytes(dir_entry, dir_entry->size, m_raw_inode.size);

    return ext2_inode;
}

size_t Ext2Inode::read_bytes(void* buffer, size_t size, size_t offset)
{
    size_t has_bytes = 0;
    size_t read_up_to = min(m_raw_inode.size, offset + size);
    size_t block_start = offset / m_block_size;
    size_t block_end = read_up_to / m_block_size;

    // Reading the first block of the block range.
    auto ext2_block = resolve_local_block(block_start);
    fs().read_block(ext2_block, fs().block_buffer());

    size_t offset_in_start_block = offset % m_block_size;
    size_t need_read_from_start_block = min(size, m_block_size - offset_in_start_block);

    memcpy(buffer, fs().block_buffer() + offset_in_start_block, need_read_from_start_block);
    has_bytes += need_read_from_start_block;

    // Reading the middle part of the block range: [start_block + 1 ; enb_block - 1]
    for (size_t block = block_start + 1; block < block_end; block++) {
        ext2_block = resolve_local_block(block);
        fs().read_block(ext2_block, buffer + has_bytes);
        has_bytes += m_block_size;
    }

    size_t left_read_from_end_block = read_up_to - offset - has_bytes;

    // Reading the last block of the block range
    if (left_read_from_end_block) {
        ext2_block = resolve_local_block(block_end);
        fs().read_block(ext2_block, fs().block_buffer());

        memcpy(buffer + has_bytes, fs().block_buffer(), left_read_from_end_block);

        has_bytes += left_read_from_end_block;
    }

    return has_bytes;
}

size_t Ext2Inode::write_bytes(void* buffer, size_t size, size_t offset)
{
    size_t has_bytes = 0;

    size_t block_start = offset / m_block_size;
    size_t block_end = (offset + size) / m_block_size;

    // Writing to the first block of the block range.
    auto ext2_block = resolve_local_block(block_start);
    fs().read_block(ext2_block, fs().block_buffer());

    size_t offset_in_start_block = offset % m_block_size;
    size_t need_write_to_start_block = min(size, m_block_size - offset_in_start_block);

    memcpy(fs().block_buffer() + offset_in_start_block, buffer, need_write_to_start_block);
    fs().write_block(ext2_block, fs().block_buffer());
    has_bytes += need_write_to_start_block;

    // Writing to the middle part of the block range: [start_block + 1 ; enb_block - 1]
    for (size_t block = block_start + 1; block < block_end; block++) {
        ext2_block = resolve_local_block(block);
        fs().write_block(ext2_block, buffer + has_bytes);
        has_bytes += m_block_size;
    }

    size_t left_write_to_end_block = size - has_bytes;

    // Writing to the last block of the block range
    if (left_write_to_end_block) {
        ext2_block = resolve_local_block(block_end);
        fs().read_block(ext2_block, fs().block_buffer());

        memcpy(fs().block_buffer(), buffer + has_bytes, left_write_to_end_block);

        fs().write_block(ext2_block, fs().block_buffer());
        has_bytes += left_write_to_end_block;
    }

    if (offset + has_bytes > m_raw_inode.size) {
        m_raw_inode.size = offset + has_bytes;
        fs().write_inode(*this);
    }

    return has_bytes;
}

size_t Ext2Inode::resolve_local_block(size_t block)
{
    size_t table_size = m_block_size / sizeof(uint32_t);

    if (block < 12) {
        if (!m_raw_inode.direct_block_pointers[block]) {
            m_raw_inode.direct_block_pointers[block] = fs().allocate_block();
            fs().write_inode(*this);
        }
        return m_raw_inode.direct_block_pointers[block];
    }
    block -= 12;

    if (block < table_size) {
        return resolve_local_block_1(m_raw_inode.singly_inderect_block_pointer, block);
    }
    block -= table_size;

    if (block < table_size * table_size) {
        return resolve_local_block_2(m_raw_inode.doubly_inderect_block_pointer, block);
    }
    block -= table_size * table_size;

    return resolve_local_block_3(m_raw_inode.triply_inderect_block_pointer, block);
}

size_t Ext2Inode::resolve_local_block_1(size_t indirection_block, size_t block)
{
    fs().read_block(indirection_block, fs().block_buffer());

    auto table = (uint32_t*)fs().block_buffer();

    if (!table[block]) {
        table[block] = fs().allocate_block();
        fs().write_block(indirection_block, fs().block_buffer());
    }

    return table[block];
}

size_t Ext2Inode::resolve_local_block_2(size_t indirection_block, size_t block)
{
    size_t table_size = m_block_size / sizeof(uint32_t);

    fs().read_block(indirection_block, fs().block_buffer());

    auto table = (uint32_t*)fs().block_buffer();
    size_t offset_in_table = block / table_size;

    if (!table[offset_in_table]) {
        table[offset_in_table] = fs().allocate_block();
        fs().write_block(indirection_block, fs().block_buffer());
    }

    return resolve_local_block_1(table[offset_in_table], block % table_size);
}

size_t Ext2Inode::resolve_local_block_3(size_t indirection_block, size_t block)
{
    size_t table_size = m_block_size / sizeof(uint32_t);

    fs().read_block(indirection_block, fs().block_buffer());

    auto table = (uint32_t*)fs().block_buffer();
    size_t offset_in_table = block / (table_size * table_size);

    if (!table[offset_in_table]) {
        table[offset_in_table] = fs().allocate_block();
        fs().write_block(indirection_block, fs().block_buffer());
    }

    return resolve_local_block_2(table[offset_in_table], block % (table_size * table_size));
}

}