#include "Ext2.hpp"

#include <Drivers/Disk/Ata.hpp>
#include <Drivers/Disk/DiskDriver.hpp>
#include <Filesystem/Base/FS.hpp>
#include <Filesystem/Base/VNode.hpp>
#include <Libkernel/Assert.hpp>
#include <Libkernel/Graphics/VgaTUI.hpp>
#include <Libkernel/Logger.hpp>

#include <Macaronlib/Bitmap.hpp>
#include <Macaronlib/Common.hpp>
#include <Macaronlib/Memory.hpp>
#include <Macaronlib/String.hpp>
#include <Macaronlib/Vector.hpp>

#define EXT2_MAGIC 0xEF53

#define SUPERBLOCK_LOCATION 1024
#define SUPERBLOCK_SIZE 1024

#define BGDT_LOCATION (SUPERBLOCK_LOCATION + SUPERBLOCK_SIZE)

namespace Kernel::FS::EXT2 {

Ext2::Ext2(Drivers::DiskDriver& disk_driver, VNodeStorage& file_storage)
    : m_disk_driver(disk_driver)
    , m_file_storage(file_storage)
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

    m_bgd_table = (block_group_descriptor_t*)malloc(m_block_size);

    m_disk_driver.read(BGDT_LOCATION / BYTES_PER_SECTOR, m_block_size / BYTES_PER_SECTOR, m_bgd_table);

    m_root = new Ext2Inode(this, 2);
    *ToExt2Inode(m_root)->inode_struct() = get_inode_structure(2);

    return true;
}

uint32_t Ext2::read(VNode& file, uint32_t offset, uint32_t size, void* buffer)
{
    return read_inode_content(ToExt2Inode(file), offset, size, buffer);
}

uint32_t Ext2::write(VNode& file, uint32_t offset, uint32_t size, void* buffer)
{
    return write_inode_content(ToExt2Inode(file), offset, size, buffer);
}

uint32_t Ext2::truncate(VNode& file, uint32_t size)
{
    auto& i_file = ToExt2Inode(file);
    const uint32_t block_start = size / m_block_size + 1; // start freeing blocks from this index
    const uint32_t block_end = i_file.inode_struct()->size / m_block_size; // end freeing blocks up to this index

    // freeing its remaining blocks
    for (size_t i = block_start; i <= block_end; i++) {
        uint32_t block_in_ext2 = resolve_inode_local_block(i_file, i);
        if (block_in_ext2) {
            free_block(block_in_ext2);
            // TODO: may be it's important to zero block pointer for inode
        }
    }

    // update file size
    i_file.inode_struct()->size = size;
    save_inode_structure(i_file);

    return size;
}

bool Ext2::can_read(VNode& vnode, uint32_t offset)
{
    return offset < ToExt2Inode(vnode).inode_struct()->size;
}

VNode* Ext2::finddir(VNode& directory, const String& filename)
{
    auto& i_directory = ToExt2Inode(directory);

    uint8_t* directory_content = (uint8_t*)malloc(i_directory.inode_struct()->size);

    read_inode_content(i_directory, 0, i_directory.inode_struct()->size, directory_content);

    size_t entry_pointer = 0;

    while (entry_pointer < i_directory.inode_struct()->size) {
        dir_entry_t entry = ((dir_entry_t*)((size_t)directory_content + entry_pointer))[0];

        char name[1024];

        memcpy(name, &((dir_entry_t*)((size_t)directory_content + entry_pointer))[0].name_characters, entry.name_len_low);
        name[entry.name_len_low] = '\0';

        if (filename == name) {
            auto* file = m_file_storage.find(this, entry.inode);
            if (!file) {
                file = new Ext2Inode(this, entry.inode);
                *ToExt2Inode(file)->inode_struct() = get_inode_structure(entry.inode);
                m_file_storage.push(file);
            }

            return file;
        }

        entry_pointer += entry.size;
    }

    free(directory_content);
    return nullptr;
}

Vector<String> Ext2::listdir(VNode& directory)
{
    auto& i_directory = ToExt2Inode(directory);

    uint8_t* directory_content = (uint8_t*)malloc(i_directory.inode_struct()->size);

    read_inode_content(i_directory, 0, i_directory.inode_struct()->size, directory_content);

    Vector<String> filenames;
    size_t entry_pointer = 0;

    while (entry_pointer < i_directory.inode_struct()->size) {
        dir_entry_t entry = ((dir_entry_t*)((size_t)directory_content + entry_pointer))[0];

        char name[1024];

        memcpy(name, &((dir_entry_t*)((size_t)directory_content + entry_pointer))[0].name_characters, entry.name_len_low);
        name[entry.name_len_low] = '\0';

        entry_pointer += entry.size;

        if (strcmp(name, ".") == 0) {
            continue;
        }

        if (strcmp(name, "..") == 0) {
            continue;
        }

        filenames.push_back(name);
    }

    free(directory_content);
    return filenames;
}

size_t Ext2::getdents(VNode& directory, linux_dirent* dirp, size_t size)
{
    auto& i_directory = ToExt2Inode(directory);
    uint8_t* directory_content = (uint8_t*)malloc(i_directory.inode_struct()->size);
    read_inode_content(i_directory, 0, i_directory.inode_struct()->size, directory_content);

    size_t ext2_dirent_pointer = 0;
    size_t linux_dirent_pointer = 0;
    linux_dirent* last_linux_dirent = nullptr;

    while (ext2_dirent_pointer < i_directory.inode_struct()->size && linux_dirent_pointer < size) {
        // Read entries by byte pointers
        dir_entry_t& ext2_entry = ((dir_entry_t*)((size_t)directory_content + ext2_dirent_pointer))[0];
        linux_dirent& linux_entry = ((linux_dirent*)((size_t)dirp + linux_dirent_pointer))[0];

        // Move ext2 pointer since ext2_entry has been read
        ext2_dirent_pointer += ext2_entry.size;

        auto size_of_linux_entry = 2 * sizeof(uint32_t) + sizeof(uint16_t) + ext2_entry.name_len_low + 1;
        // Out of memory in linux_dirent buffer
        if (linux_dirent_pointer + size_of_linux_entry > size) {
            break;
        }

        // Get filename
        char name[256];
        memcpy(name, &ext2_entry.name_characters, ext2_entry.name_len_low);
        name[ext2_entry.name_len_low] = '\0';

        // Skip Ext2 irrelevant entries
        if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
            continue;
        }

        memcpy(linux_entry.d_name, name, ext2_entry.name_len_low + 1);

        // Get inode
        linux_entry.d_ino = ext2_entry.inode;
        linux_entry.d_reclen = size_of_linux_entry;
        linux_dirent_pointer += linux_entry.d_reclen;

        // Set offset to the next entry
        if (last_linux_dirent) {
            last_linux_dirent->d_off = (size_t)&linux_entry - (size_t)last_linux_dirent;
        }

        last_linux_dirent = &linux_entry;
    }

    return linux_dirent_pointer;
}

VNode* Ext2::create(VNode& directory, const String& name, FileType type, FilePermissions perms)
{
    // TODO: what if file already exists? Probably should be checked at the VFS layer

    // init file inode structure, then save it
    uint32_t free_inode = occypy_inode();
    auto created_file = new Ext2Inode(this, free_inode);

    created_file->inode_struct()->type_and_permissions = static_cast<uint16_t>(type) | perms;
    created_file->inode_struct()->user_id = 1;
    created_file->inode_struct()->size = 0;
    created_file->inode_struct()->c_time = 116;

    m_file_storage.push(created_file);
    save_inode_structure(created_file);

    // init directory entry, then append it
    uint32_t file_entry_size = sizeof(dir_entry_t) + name.size();
    dir_entry_t* file_entry = (dir_entry_t*)malloc(file_entry_size);

    file_entry[0].inode = free_inode;
    file_entry[0].size = file_entry_size;
    file_entry[0].name_len_low = name.size();

    for (size_t i = 0; i < name.size(); i++) {
        file_entry[0].name_characters[i] = name[i];
    }

    write_inode_content(ToExt2Inode(directory), ToExt2Inode(directory).inode_struct()->size, file_entry_size, file_entry);

    return created_file;
}

bool Ext2::erase(VNode& directory, const VNode& file)
{
    auto& i_directory = ToExt2Inode(directory);
    uint8_t* directory_content = (uint8_t*)malloc(i_directory.inode_struct()->size);

    read_inode_content(i_directory, 0, i_directory.inode_struct()->size, directory_content);

    size_t entry_pointer = 0;

    while (entry_pointer < i_directory.inode_struct()->size) {
        dir_entry_t entry = ((dir_entry_t*)((size_t)directory_content + entry_pointer))[0];

        if (file.vnode() == entry.inode) {
            // mark file's inode as free
            free_inode(file.vnode());

            // delete file's row from dir_entry table
            i_directory.inode_struct()->size -= entry.size;
            save_inode_structure(i_directory);
            write_inode_content(i_directory, entry_pointer, i_directory.inode_struct()->size - entry_pointer - entry.size, (void*)(entry_pointer + entry.size));

            free(directory_content);
            return true;
        }

        entry_pointer += entry.size;
    }

    free(directory_content);
    return false;
}

bool Ext2::read_blocks(uint32_t block, uint32_t block_size, void* mem)
{
    return m_disk_driver.read(block * m_block_size / BYTES_PER_SECTOR, block_size * m_block_size / BYTES_PER_SECTOR, mem);
}

uint32_t Ext2::allocate_block()
{
    return occypy_block();
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

void Ext2::write_vnode(VNode& inode)
{
    save_inode_structure(ToExt2Inode(inode));
}

uint32_t Ext2::allocate_inode()
{
    return occypy_inode();
}

uint32_t Ext2::resolve_inode_local_block(Ext2Inode& i_file, uint32_t block, bool need_create)
{
    const size_t table_size = m_block_size / 4;

    if (block < 12) {
        if (!i_file.inode_struct()->direct_block_pointers[block]) {
            if (need_create) {
                i_file.inode_struct()->direct_block_pointers[block] = occypy_block();
                save_inode_structure(i_file);
            }
        }
        return i_file.inode_struct()->direct_block_pointers[block];
    }
    block -= 12;

    if (block < table_size) {
        if (!i_file.inode_struct()->singly_inderect_block_pointer) {
            if (need_create) {
                i_file.inode_struct()->singly_inderect_block_pointer = occypy_block();
                save_inode_structure(i_file);
            } else {
                return 0;
            }
        }
        uint32_t table[table_size];
        read_block(i_file.inode_struct()->singly_inderect_block_pointer, &table);
        if (!table[block]) {
            if (need_create) {
                table[block] = occypy_block();
                write_block(i_file.inode_struct()->singly_inderect_block_pointer, &table);
            } else {
                return 0;
            }
        }
        return table[block];
    }
    block -= 256;

    if (block < table_size * table_size) {
        if (!i_file.inode_struct()->doubly_inderect_block_pointer) {
            if (need_create) {
                i_file.inode_struct()->doubly_inderect_block_pointer = occypy_block();
                save_inode_structure(i_file);
            } else {
                return 0;
            }
        }
        uint32_t double_table[table_size];
        read_block(i_file.inode_struct()->doubly_inderect_block_pointer, &double_table);

        if (!double_table[block / table_size]) {
            if (need_create) {
                double_table[block / table_size] = occypy_block();
                write_block(i_file.inode_struct()->doubly_inderect_block_pointer, &double_table);
            } else {
                return 0;
            }
        }
        uint32_t table[table_size];
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
        if (!i_file.inode_struct()->triply_inderect_block_pointer) {
            if (need_create) {
                i_file.inode_struct()->triply_inderect_block_pointer = occypy_block();
                save_inode_structure(i_file);
            } else {
                return 0;
            }
        }
        uint32_t triple_table[table_size];
        read_block(i_file.inode_struct()->triply_inderect_block_pointer, &triple_table);

        if (!triple_table[block / (table_size * table_size)]) {
            if (need_create) {
                triple_table[block / (table_size * table_size)] = occypy_block();
                write_block(i_file.inode_struct()->triply_inderect_block_pointer, &triple_table);
            } else {
                return 0;
            }
        }
        uint32_t double_table[table_size];
        read_block(triple_table[block / (table_size * table_size)], &double_table);

        if (!double_table[block / table_size]) {
            if (need_create) {
                double_table[block / table_size] = occypy_block();
                write_block(triple_table[block / (table_size * table_size)], &double_table);
            } else {
                return 0;
            }
        }

        uint32_t table[table_size];
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

uint32_t Ext2::read_inode_content(Ext2Inode& file, uint32_t offset, uint32_t size, void* mem)
{
    if (offset >= file.inode_struct()->size) {
        return 0;
    }

    const uint32_t block_start = offset / m_block_size;
    const uint32_t block_end = min(file.inode_struct()->size, offset + size) / m_block_size;

    uint32_t read_bytes = 0;

    // reading left part
    auto ext2_block = resolve_inode_local_block(file, block_start);

    read_block(ext2_block, m_block_buffer);
    memcpy(mem, m_block_buffer + offset % m_block_size, min(size, m_block_size - (offset % m_block_size)));

    read_bytes += min(size, m_block_size - (offset % m_block_size));

    // reading middle part
    if (block_end > 0) {
        for (size_t block = block_start + 1; block < block_end; block++) {
            ext2_block = resolve_inode_local_block(file, block);
            read_block(ext2_block, (void*)((uint32_t)mem + read_bytes));
            read_bytes += m_block_size;
        }
    }

    // reading right part
    if (block_start != block_end) {
        ext2_block = resolve_inode_local_block(file, block_end);
        read_block(ext2_block, m_block_buffer);
        memcpy((void*)((uint32_t)mem + read_bytes), m_block_buffer, (offset + size) % m_block_size);
        read_bytes += (offset + size) % m_block_size;
    }

    return read_bytes;
}

uint32_t Ext2::write_inode_content(Ext2Inode& file, uint32_t offset, uint32_t size, void* mem)
{
    const uint32_t block_start = offset / m_block_size;
    const uint32_t block_end = (offset + size) / m_block_size;

    // writing to the left part block
    uint32_t left_tail_block = resolve_inode_local_block(file, block_start, true);
    read_block(left_tail_block, m_block_buffer);
    memcpy(m_block_buffer + offset % m_block_size, mem, min(size, m_block_size - (offset % m_block_size)));
    write_block(left_tail_block, m_block_buffer);

    uint32_t written_bytes = min(size, m_block_size - (offset % m_block_size));

    // reading middle part
    if (block_end > 0) {
        for (size_t block = block_start + 1; block < block_end; block++) {
            uint32_t middle_block = resolve_inode_local_block(file, block, true);
            read_block(middle_block, m_block_buffer);
            memcpy(m_block_buffer, (void*)((uint32_t)mem + written_bytes), m_block_size);
            write_block(middle_block, m_block_buffer);

            written_bytes += m_block_size;
        }
    }

    // reading right part
    if (block_start != block_end) {
        uint32_t right_tail_block = resolve_inode_local_block(file, block_end, true);

        read_block(right_tail_block, m_block_buffer);
        memcpy(m_block_buffer, (void*)((uint32_t)mem + written_bytes), (offset + size) % m_block_size);
        write_block(right_tail_block, m_block_buffer);
    }

    // update file size
    if (offset + written_bytes > file.inode_struct()->size) {
        file.inode_struct()->size = offset + written_bytes;
    }

    save_inode_structure(file);

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

bool Ext2::save_inode_structure(Ext2Inode& file)
{
    uint32_t block_group_index = (file.vnode() - 1) / m_superblock.inodes_per_block_group;
    uint32_t inode_table_block = m_bgd_table[block_group_index].inode_table_addr;
    uint32_t inode_table_index = (file.vnode() - 1) % m_superblock.inodes_per_block_group;

    // block, where inode structure is stored
    uint32_t inode_block = inode_table_block + (inode_table_index * sizeof(inode_t)) / m_block_size;

    // (index) position of inode structure inside inode block
    uint32_t inode_block_index = inode_table_index % (m_block_size / sizeof(inode_t));

    // copying block, where inode structure is stored
    inode_t* inodes = new inode_t[m_block_size / sizeof(inode_t)];
    read_block(inode_block, inodes);

    // saving new inode structure
    inodes[inode_block_index] = *file.inode_struct();
    bool result = write_block(inode_block, inodes);

    delete[] inodes;

    return result;
}

bool Ext2::save_inode_structure(Ext2Inode* file)
{
    if (file) {
        return save_inode_structure(*file);
    }
    return false;
}

uint32_t Ext2::occypy_block(uint32_t preferd_block_group, bool fill_zeroes)
{
    for (size_t i = preferd_block_group; i < m_bgd_table_size; i++) {
        if (m_bgd_table[i].unallocated_block_count) {
            // read bitmap
            uint32_t* block_bitmap = (uint32_t*)malloc(m_block_size);
            read_block(m_bgd_table[i].block_bitmap_addr, block_bitmap);

            // find first free block in bitmap
            Bitmap bit = Bitmap::wrap((uint32_t)block_bitmap, m_superblock.blocks_per_block_group);
            const uint32_t free_block_in_bitmap = bit.find_first_zero();

            // occupy block
            const uint32_t free_block_in_ext2 = i * m_superblock.blocks_per_block_group + free_block_in_bitmap + 1;
            bit.set_true(free_block_in_bitmap);
            write_block(m_bgd_table[i].block_bitmap_addr, block_bitmap);

            // clear block
            if (fill_zeroes) {
                uint8_t* block_buffer = (uint8_t*)malloc(m_block_size);
                memset(block_buffer, 0, m_block_size);
                write_block(free_block_in_ext2, block_buffer);
                free(block_buffer);
            }

            return free_block_in_ext2;
        }
    }

    return 0;
}

bool Ext2::free_block(uint32_t block)
{
    uint32_t* block_bitmap = (uint32_t*)malloc(m_block_size);
    read_block(m_bgd_table[(block - 1) / m_superblock.blocks_per_block_group].block_bitmap_addr, block_bitmap);
    Bitmap bit = Bitmap::wrap((uint32_t)block_bitmap, m_superblock.blocks_per_block_group);
    bit.set_false((block - 1) % m_superblock.inodes_per_block_group);
    return write_block(m_bgd_table[(block - 1) / m_superblock.blocks_per_block_group].block_bitmap_addr, block_bitmap);
}

uint32_t Ext2::occypy_inode(uint32_t preferd_block_group)
{
    for (size_t i = preferd_block_group; i < m_bgd_table_size; i++) {
        if (m_bgd_table[i].unallocated_inodes_count) {
            // read bitmap
            uint32_t* inode_bitmap = (uint32_t*)malloc(m_block_size);
            read_block(m_bgd_table[i].inode_bitmap_addr, inode_bitmap);

            // find first free inode in bitmap
            Bitmap bit = Bitmap::wrap((uint32_t)inode_bitmap, m_superblock.inodes_per_block_group);
            const uint32_t free_inode = bit.find_first_zero();

            // occupy inode
            bit.set_true(free_inode);
            write_block(m_bgd_table[i].inode_bitmap_addr, inode_bitmap);

            return i * m_superblock.inodes_per_block_group + free_inode + 1; // inodes starts from 1
        }
    }

    return 0;
}

bool Ext2::free_inode(uint32_t inode)
{
    uint32_t* inode_bitmap = (uint32_t*)malloc(m_block_size);
    read_block(m_bgd_table[(inode - 1) / m_superblock.inodes_per_block_group].inode_bitmap_addr, inode_bitmap);
    Bitmap bit = Bitmap::wrap((uint32_t)inode_bitmap, m_superblock.inodes_per_block_group);
    bit.set_false((inode - 1) % m_superblock.inodes_per_block_group);
    return write_block(m_bgd_table[(inode - 1) / m_superblock.inodes_per_block_group].inode_bitmap_addr, inode_bitmap);
}
}
