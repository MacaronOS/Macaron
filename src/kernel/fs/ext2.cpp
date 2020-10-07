#include "ext2.hpp"
#include "File.hpp"

#include "../algo/Bitmap.hpp"
#include "../algo/String.hpp"
#include "../algo/Vector.hpp"
#include "../assert.hpp"
#include "../drivers/disk/Ata.hpp"
#include "../memory/kmalloc.hpp"
#include "../memory/memory.hpp"
#include "../monitor.hpp"

#define EXT2_MAGIC 0xEF53

#define SUPERBLOCK_LOCATION 1024
#define SUPERBLOCK_SIZE 1024

#define BGDT_LOCATION (SUPERBLOCK_LOCATION + SUPERBLOCK_SIZE)

namespace kernel::fs::ext2 {

Ext2::Ext2(drivers::DiskDriver& disk_driver)
    : m_disk_driver(disk_driver)
{
}

Ext2::~Ext2()
{
    delete[] m_bgd_table;
    delete[] m_block_buffer;
    delete[] m_table_buffer_1;
    delete[] m_table_buffer_2;
    delete[] m_table_buffer_3;
}

bool Ext2::init()
{
    m_disk_driver.read(SUPERBLOCK_LOCATION / BYTES_PER_SECTOR, SUPERBLOCK_SIZE / BYTES_PER_SECTOR, &m_superblock);

    if (m_superblock.magic != EXT2_MAGIC) {
        return false;
    }
    m_block_size = (1024 << m_superblock.block_shift);

    // allocating space for buffers
    m_block_buffer = new char[m_block_size];
    m_table_buffer_1 = new char[m_block_size / 4];
    m_table_buffer_2 = new char[m_block_size / 4];
    m_table_buffer_3 = new char[m_block_size / 4];

    m_bgd_table_size = (m_superblock.blocks_count + m_superblock.blocks_per_block_group - 1) / m_superblock.blocks_per_block_group;

    m_bgd_table = (block_group_descriptor_t*)kmalloc(m_block_size);

    m_disk_driver.read(BGDT_LOCATION / BYTES_PER_SECTOR, m_block_size / BYTES_PER_SECTOR, m_bgd_table);

    // initialising the root
    m_root = File(2, "ext2");
    m_root.set_type(FileType::Directory);
    m_root.set_permission(FilePermission::Read);
    m_root.bind_fs(this);

    return true;
}

uint32_t Ext2::read(const File& file, uint32_t offset, uint32_t size, void* buffer)
{
    inode_cache_t inode_cache = { file.inode(), get_inode_structure(file.inode()) };
    return read_inode_content(&inode_cache, offset, size, buffer);
}

uint32_t Ext2::write(const File& file, uint32_t offset, uint32_t size, void* buffer)
{
    inode_cache_t inode_cache = { file.inode(), get_inode_structure(file.inode()) };
    return write_inode_content(&inode_cache, offset, size, buffer);
}

File* Ext2::finddir(const File& directory, const String& filename)
{
    inode_cache_t inode_cache = { directory.inode(), get_inode_structure(directory.inode()) };
    uint8_t* inode_content = (uint8_t*)kmalloc(inode_cache.inode_struct.size);

    read_inode_content(&inode_cache, 0, inode_cache.inode_struct.size, inode_content);

    size_t entry_pointer = 0;

    while (entry_pointer < inode_cache.inode_struct.size) {
        dir_entry_t entry = ((dir_entry_t*)(inode_content + entry_pointer))[0];

        char name[1024];

        memcpy(name, &((dir_entry_t*)(inode_content + entry_pointer))[0].name_characters, entry.name_len_low);
        name[entry.name_len_low] = '\0';

        if (filename == name) {
            inode_t file_inode_struct = get_inode_structure(entry.inode);
            File* file = new File(entry.inode, filename);
            file->set_type(static_cast<FileType>(file_inode_struct.type_and_permissions & 0xF000));
            file->bind_fs(this);

            kfree(inode_content);
            return file;
        }

        entry_pointer += entry.size;
    }

    kfree(inode_content);
    return nullptr;
}

Vector<File*> Ext2::listdir(const File& directory)
{
    inode_cache_t inode_cache = { directory.inode(), get_inode_structure(directory.inode()) };
    uint8_t* inode_content = (uint8_t*)kmalloc(inode_cache.inode_struct.size);

    read_inode_content(&inode_cache, 0, inode_cache.inode_struct.size, inode_content);

    Vector<File*> files;
    size_t entry_pointer = 0;

    while (entry_pointer < inode_cache.inode_struct.size) {
        dir_entry_t entry = ((dir_entry_t*)(inode_content + entry_pointer))[0];

        char name[1024];

        memcpy(name, &((dir_entry_t*)(inode_content + entry_pointer))[0].name_characters, entry.name_len_low);
        name[entry.name_len_low] = '\0';

        inode_t file_inode_struct = get_inode_structure(entry.inode);
        File* file = new File(entry.inode, name);
        file->set_type(static_cast<FileType>(file_inode_struct.type_and_permissions & 0xF000));
        file->bind_fs(this);
        files.push_back(file);

        entry_pointer += entry.size;
    }

    kfree(inode_content);
    return files;
}

File& Ext2::create(const File& directory, File& file)
{
    // get inode content of the directory
    inode_cache_t directory_inode_cache = { directory.inode(), get_inode_structure(directory.inode()) };

    // init file inode structure, then save it
    inode_cache_t file_inode_cache;
    file_inode_cache.inode = occypy_inode();
    file_inode_cache.inode_struct.type_and_permissions = file.get_permissions() | static_cast<uint16_t>(file.type());
    file_inode_cache.inode_struct.user_id = 1;
    file_inode_cache.inode_struct.size = file.size();
    file_inode_cache.inode_struct.c_time = 116;
    save_inode_structure(&file_inode_cache);

    // init directory entry, then append it
    uint32_t file_entry_size = sizeof(dir_entry_t) - 1 + file.name().size();
    dir_entry_t* file_entry = (dir_entry_t*)kmalloc(file_entry_size);
    file_entry[0].inode = file_inode_cache.inode;
    file_entry[0].size = file_entry_size;
    file_entry[0].name_len_low = file.name().size();

    for (size_t i = 0; i < file.name().size(); i++) {
        *(&file_entry[0].name_characters + i) = file.name()[i];
    }

    write_inode_content(&directory_inode_cache, directory_inode_cache.inode_struct.size, file_entry_size, file_entry);
    directory_inode_cache.inode_struct.size += file_entry_size;
    save_inode_structure(&directory_inode_cache);

    file.set_inode(file_inode_cache.inode);

    return file;
}

bool Ext2::erase(const File& directory, const File& file)
{
    inode_cache_t inode_cache = { directory.inode(), get_inode_structure(directory.inode()) };
    uint8_t* inode_content = (uint8_t*)kmalloc(inode_cache.inode_struct.size);

    read_inode_content(&inode_cache, 0, inode_cache.inode_struct.size, inode_content);

    size_t entry_pointer = 0;

    while (entry_pointer < inode_cache.inode_struct.size) {
        dir_entry_t entry = ((dir_entry_t*)(inode_content + entry_pointer))[0];

        if (file.inode() == entry.inode) {
            free_inode(file.inode());
            inode_cache.inode_struct.size -= entry.size;
            save_inode_structure(&inode_cache);
            write_inode_content(&inode_cache, entry_pointer, inode_cache.inode_struct.size - entry_pointer - entry.size, (void*)(entry_pointer + entry.size));
            kfree(inode_content);
            return true;
        }

        entry_pointer += entry.size;
    }

    kfree(inode_content);
    return false;
}

bool Ext2::read_blocks(uint32_t block, uint32_t block_size, void* mem)
{
    return m_disk_driver.read(block * m_block_size / BYTES_PER_SECTOR, block_size * m_block_size / BYTES_PER_SECTOR, mem);
}

bool Ext2::read_block(uint32_t block, void* mem)
{
    return read_blocks(block, 1, mem);
}

bool Ext2::write_blocks(uint32_t block, uint32_t block_size, void* mem)
{
    return m_disk_driver.write(block * m_block_size / BYTES_PER_SECTOR, block_size * m_block_size / BYTES_PER_SECTOR, mem);
}

bool Ext2::write_block(uint32_t block, void* mem)
{
    return write_blocks(block, 1, mem);
}

uint32_t Ext2::resolve_inode_local_block(inode_cache_t* inode, uint32_t block, bool need_create)
{
    const int table_size = m_block_size / 4;

    if (block < 12) {
        if (!inode->inode_struct.direct_block_pointers[block]) {
            if (need_create) {
                inode->inode_struct.direct_block_pointers[block] = occypy_block();
                save_inode_structure(inode);
            }
        }
        return inode->inode_struct.direct_block_pointers[block];
    }
    block -= 12;

    if (block < table_size) {
        if (!inode->inode_struct.singly_inderect_block_pointer) {
            if (need_create) {
                inode->inode_struct.singly_inderect_block_pointer = occypy_block();
                save_inode_structure(inode);
            } else {
                return 0;
            }
        }
        uint8_t table[table_size];
        read_block(inode->inode_struct.singly_inderect_block_pointer, &table);
        if (!table[block]) {
            if (need_create) {
                table[block] = occypy_block();
                write_block(inode->inode_struct.singly_inderect_block_pointer, &table);
            } else {
                return 0;
            }
        }
        return table[block];
    }
    block -= 256;

    if (block < table_size * table_size) {
        if (!inode->inode_struct.doubly_inderect_block_pointer) {
            if (need_create) {
                inode->inode_struct.doubly_inderect_block_pointer = occypy_block();
                save_inode_structure(inode);
            } else {
                return 0;
            }
        }
        uint8_t double_table[table_size];
        read_block(inode->inode_struct.doubly_inderect_block_pointer, &double_table);

        if (!double_table[block / table_size]) {
            if (need_create) {
                double_table[block / table_size] = occypy_block();
                write_block(inode->inode_struct.doubly_inderect_block_pointer, &double_table);
            } else {
                return 0;
            }
        }
        uint8_t table[table_size];
        read_block(double_table[block / table_size], &table);

        if (!table[block % table_size]) {
            if (need_create) {
                table[block % table_size] = occypy_block();
                write_block(double_table[block / table_size], &table);
            } else {
                return 0;
            }
        }

        return table[block % table_size];
    }
    block -= 256 * 256;

    if (block < table_size * table_size * table_size) {
        if (!inode->inode_struct.triply_inderect_block_pointer) {
            if (need_create) {
                inode->inode_struct.triply_inderect_block_pointer = occypy_block();
                save_inode_structure(inode);
            } else {
                return 0;
            }
        }
        uint8_t triple_table[table_size];
        read_block(inode->inode_struct.triply_inderect_block_pointer, &triple_table);

        if (!triple_table[block / (table_size * table_size)]) {
            if (need_create) {
                triple_table[block / (table_size * table_size)] = occypy_block();
                write_block(inode->inode_struct.triply_inderect_block_pointer, &triple_table);
            } else {
                return 0;
            }
        }
        uint8_t double_table[table_size];
        read_block(triple_table[block / (table_size * table_size)], &double_table);

        if (!double_table[block / table_size]) {
            if (need_create) {
                double_table[block / table_size] = occypy_block();
                write_block(triple_table[block / (table_size * table_size)], &double_table);
            } else {
                return 0;
            }
        }

        uint8_t table[table_size];
        read_block(double_table[block / table_size], &table);

        if (!table[block % table_size]) {
            if (need_create) {
                table[block % table_size] = occypy_block();
                write_block(double_table[block / table_size], &table);
            } else {
                return 0;
            }
        }

        return table[block % table_size];
    }

    return 0;
}

uint32_t Ext2::read_inode_content(inode_cache_t* inode, uint32_t offset, uint32_t size, void* mem)
{
    const uint32_t block_start = offset / m_block_size;
    const uint32_t block_end = (offset + size) / m_block_size;

    // reading left part
    read_block(resolve_inode_local_block(inode, block_start), m_block_buffer);
    memcpy(mem, m_block_buffer + offset % m_block_size, min(size, m_block_size - (offset % m_block_size)));

    uint32_t read_bytes = min(size, m_block_size - (offset % m_block_size));

    // reading middle part
    if (block_end > 0) {
        for (size_t block = block_start + 1; block < block_end - 1; block++) {
            read_block(resolve_inode_local_block(inode, block), mem + read_bytes);
            read_bytes += m_block_size;
        }
    }

    // reading right part
    if (block_start != block_end) {
        read_block(resolve_inode_local_block(inode, block_end), m_block_buffer);
        memcpy(mem + read_bytes, m_block_buffer, (offset + size) % m_block_size);
    }

    return size;
}

uint32_t Ext2::write_inode_content(inode_cache_t* inode, uint32_t offset, uint32_t size, void* mem)
{
    const uint32_t block_start = offset / m_block_size;
    const uint32_t block_end = (offset + size) / m_block_size;

    // writing to the left part block
    uint32_t left_tail_block = resolve_inode_local_block(inode, block_start, true);
    read_block(left_tail_block, m_block_buffer);
    memcpy(m_block_buffer + offset % m_block_size, mem, min(size, m_block_size - (offset % m_block_size)));
    write_block(left_tail_block, m_block_buffer);

    uint32_t written_bytes = min(size, m_block_size - (offset % m_block_size));

    // reading middle part
    if (block_end > 0) {
        for (size_t block = block_start + 1; block < block_end - 1; block++) {
            uint32_t middle_block = resolve_inode_local_block(inode, block, true);
            read_block(middle_block, m_block_buffer);
            memcpy(m_block_buffer, mem + written_bytes, m_block_size);
            write_block(middle_block, m_block_buffer);

            written_bytes += m_block_size;
        }
    }

    // reading right part
    if (block_start != block_end) {
        uint32_t right_tail_block = resolve_inode_local_block(inode, block_end, true);

        read_block(right_tail_block, m_block_buffer);
        memcpy(m_block_buffer, mem + written_bytes, (offset + size) % m_block_size);
        write_block(right_tail_block, m_block_buffer);
    }

    return size;
}

inode_t Ext2::get_inode_structure(uint32_t inode)
{
    uint32_t block_group_index = (inode - 1) / m_superblock.inodes_per_block_group;
    uint32_t inode_table_block = m_bgd_table[block_group_index].inode_table_addr;
    uint32_t inode_table_index = (inode - 1) % m_superblock.inodes_per_block_group;

    // block, where inode structure is stored
    uint32_t inode_block = inode_table_block + (inode_table_index * sizeof(inode_t)) / m_block_size;

    // (index) position of inode structure inside inode block
    uint32_t inode_block_index = inode_table_index % (m_block_size / sizeof(inode_t));

    // copying block, where inode structure is stored
    inode_t* inodes = new inode_t[m_block_size / sizeof(inode_t)];
    read_block(inode_block, inodes);

    inode_t result = inodes[inode_block_index];

    delete[] inodes;

    return result;
}

bool Ext2::save_inode_structure(inode_cache_t* inode)
{
    uint32_t block_group_index = (inode->inode - 1) / m_superblock.inodes_per_block_group;
    uint32_t inode_table_block = m_bgd_table[block_group_index].inode_table_addr;
    uint32_t inode_table_index = (inode->inode - 1) % m_superblock.inodes_per_block_group;

    // block, where inode structure is stored
    uint32_t inode_block = inode_table_block + (inode_table_index * sizeof(inode_t)) / m_block_size;

    // (index) position of inode structure inside inode block
    uint32_t inode_block_index = inode_table_index % (m_block_size / sizeof(inode_t));

    // copying block, where inode structure is stored
    inode_t* inodes = new inode_t[m_block_size / sizeof(inode_t)];
    read_block(inode_block, inodes);

    // saving new inode structure
    inodes[inode_block_index] = inode->inode_struct;
    bool result = write_block(inode_block, inodes);

    delete[] inodes;

    return result;
}

uint32_t Ext2::occypy_block(uint32_t preferd_block_group, bool fill_zeroes)
{
    for (size_t i = preferd_block_group; i < m_bgd_table_size; i++) {
        if (m_bgd_table[i].unallocated_block_count) {
            // read bitmap
            uint32_t* block_bitmap = (uint32_t*)kmalloc(m_block_size);
            read_block(m_bgd_table[i].block_bitmap_addr, block_bitmap);

            // find first free block in bitmap
            Bitmap bit = Bitmap::wrap((uint32_t)block_bitmap, m_superblock.blocks_per_block_group);
            uint32_t free_block = bit.find_first_zero();

            // occupy block
            bit.set_true(free_block);
            write_block(m_bgd_table[i].block_bitmap_addr, block_bitmap);

            // clear block
            if (fill_zeroes) {
                uint8_t* block_buffer = (uint8_t*)kmalloc(m_block_size);
                memset(block_buffer, 0, m_block_size);
                write_block(free_block, block_buffer);
                kfree(block_buffer);
            }

            return free_block;
        }
    }

    return 0;
}

uint32_t Ext2::occypy_inode(uint32_t preferd_block_group)
{
    for (size_t i = preferd_block_group; i < m_bgd_table_size; i++) {
        if (m_bgd_table[i].unallocated_inodes_count) {
            // read bitmap
            uint32_t* inode_bitmap = (uint32_t*)kmalloc(m_block_size);
            read_block(m_bgd_table[i].inode_bitmap_addr, inode_bitmap);

            // find first free inode in bitmap
            Bitmap bit = Bitmap::wrap((uint32_t)inode_bitmap, m_superblock.inodes_per_block_group);
            uint32_t free_inode = bit.find_first_zero();

            // occupy inode
            bit.set_true(free_inode);
            write_block(m_bgd_table[i].inode_bitmap_addr, inode_bitmap);

            return free_inode + 1; // inodes starts from 1
        }
    }

    return 0;
}

bool Ext2::free_inode(uint32_t inode)
{
    uint32_t* inode_bitmap = (uint32_t*)kmalloc(m_block_size);
    read_block(m_bgd_table[(inode - 1) / m_superblock.inodes_per_block_group].inode_bitmap_addr, inode_bitmap);
    Bitmap bit = Bitmap::wrap((uint32_t)inode_bitmap, m_superblock.inodes_per_block_group);
    bit.set_false((inode - 1) % m_superblock.inodes_per_block_group);
    return write_block(m_bgd_table[(inode - 1) / m_superblock.inodes_per_block_group].inode_bitmap_addr, inode_bitmap);
}

// test funcs
void Ext2::read_directory(uint32_t inode)
{
    inode_cache_t inode_cache = { inode, get_inode_structure(inode) };
    uint8_t* inode_content = (uint8_t*)kmalloc(inode_cache.inode_struct.size);

    // read_inode_content(&inode_struct, inode_content);
    read_inode_content(&inode_cache, 0, inode_cache.inode_struct.size, inode_content);

    size_t entry_pointer = 0;

    while (entry_pointer < inode_cache.inode_struct.size) {
        uint32_t name_size = (((dir_entry_t*)(inode_content + entry_pointer))[0].name_len_low);
        char name[1024];
        memcpy(name, &((dir_entry_t*)(inode_content + entry_pointer))[0].name_characters, name_size);
        name[name_size] = '\0';
        term_print(name);
        term_print(" : ");
        term_printd((((dir_entry_t*)(inode_content + entry_pointer))[0].inode));
        term_print("\n");

        entry_pointer += ((dir_entry_t*)(inode_content + entry_pointer))[0].size;
    }
}

void Ext2::read_inode(uint32_t inode)
{
    inode_cache_t inode_cache = { inode, get_inode_structure(inode) };
    char* inode_content = (char*)kmalloc(inode_cache.inode_struct.size + 1);

    read_inode_content(&inode_cache, 0, inode_cache.inode_struct.size, inode_content);

    inode_content[inode_cache.inode_struct.size] = '\0';

    term_print(inode_content);
}

}
