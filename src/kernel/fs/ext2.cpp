#include "ext2.hpp"
#include "File.hpp"

#include "../algo/String.hpp"
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
    if (m_bgd_table) {
        delete[] m_bgd_table;
    }
    if (m_tails_buffer) {
        delete[] m_tails_buffer;
    }
    if (m_table_buffer_1) {
        delete[] m_table_buffer_1;
    }
    if (m_table_buffer_2) {
        delete[] m_table_buffer_2;
    }
    if (m_table_buffer_3) {
        delete[] m_table_buffer_3;
    }
}

bool Ext2::init()
{
    m_disk_driver.read(SUPERBLOCK_LOCATION / BYTES_PER_SECTOR, SUPERBLOCK_SIZE / BYTES_PER_SECTOR, &m_superblock);

    if (m_superblock.magic != EXT2_MAGIC) {
        return false;
    }
    m_block_size = (1024 << m_superblock.block_shift);

    // allocating space for buffers
    m_tails_buffer = new char[m_block_size];
    m_table_buffer_1 = new char[m_block_size / 4];
    m_table_buffer_2 = new char[m_block_size / 4];
    m_table_buffer_3 = new char[m_block_size / 4];

    m_bgd_table_size = (m_superblock.blocks_count + m_superblock.blocks_per_block_group - 1) / m_superblock.blocks_per_block_group;

    m_bgd_table = (block_group_descriptor_t*)kmalloc(m_block_size);

    m_disk_driver.read(BGDT_LOCATION / BYTES_PER_SECTOR, m_block_size / BYTES_PER_SECTOR, m_bgd_table);

    return true;
}

uint32_t Ext2::read(File& file, uint32_t offset, uint32_t size, void* buffer)
{
    inode_t inode_struct = get_inode_structure(file.inode());
    return read_inode_content(&inode_struct, offset, size, buffer);
}

File Ext2::finddir(const File& directory, const String& filename)
{
    inode_t inode_struct = get_inode_structure(directory.inode());
    uint8_t* inode_content = (uint8_t*)kmalloc(inode_struct.size);

    read_inode_content(&inode_struct, 0, inode_struct.size, inode_content);

    size_t entry_pointer = 0;

    while (entry_pointer < inode_struct.size) {
        dir_entry_t entry = ((dir_entry_t*)(inode_content + entry_pointer))[0];

        char name[1024];

        memcpy(name, &((dir_entry_t*)(inode_content + entry_pointer))[0].name_characters, entry.name_len_low);
        name[entry.name_len_low] = '\0';

        if (filename == name) {
            return File(entry.inode, filename);
        }

        entry_pointer += entry.size;
    }

    return File(FileType::NOTAFILE);
}

bool Ext2::read_blocks(uint32_t block, uint32_t block_size, void* mem)
{
    return m_disk_driver.read(block * m_block_size / BYTES_PER_SECTOR, block_size * m_block_size / BYTES_PER_SECTOR, mem);
}

bool Ext2::read_block(uint32_t block, void* mem)
{
    return read_blocks(block, 1, mem);
}

uint32_t Ext2::resolve_inode_local_block(inode_t* inode, uint32_t block)
{
    const int table_size = m_block_size / 4;

    if (block < 12) {
        return inode->direct_block_pointers[block];
    }
    block -= 12;

    if (block < table_size) {
        uint8_t table[table_size];
        read_block(inode->singly_inderect_block_pointer, &table);
        return table[block];
    }
    block -= 256;

    if (block < table_size * table_size) {
        uint8_t double_table[table_size];
        read_block(inode->doubly_inderect_block_pointer, &double_table);

        uint8_t table[table_size];
        read_block(double_table[block / table_size], &table);

        return table[block % table_size];
    }
    block -= 256 * 256;

    if (block < table_size * table_size * table_size) {
        uint8_t triple_table[table_size];
        read_block(inode->triply_inderect_block_pointer, &triple_table);

        uint8_t double_table[table_size];
        read_block(triple_table[block / (table_size * table_size)], &double_table);

        uint8_t table[table_size];
        read_block(double_table[block / table_size], &table);

        return table[block % table_size];
    }

    return -1;
}

uint32_t Ext2::read_inode_content(inode_t* inode, uint32_t offset, uint32_t size, void* mem)
{
    const uint32_t block_start = offset / m_block_size;
    const uint32_t block_end = (offset + size) / m_block_size;

    // reading left part
    read_block(resolve_inode_local_block(inode, block_start), m_tails_buffer);
    memcpy(mem, m_tails_buffer + offset % m_block_size, min(size, m_block_size - (offset % m_block_size)));

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
        read_block(resolve_inode_local_block(inode, block_end), m_tails_buffer);
        memcpy(mem + read_bytes, m_tails_buffer, (offset + size) % m_block_size);
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
    inode_t inodes[m_block_size / sizeof(inode_t)];
    m_disk_driver.read(inode_block * 2, 2, &inodes);

    return inodes[inode_block_index];
}

// test funcs
void Ext2::read_directory(uint32_t inode)
{
    inode_t inode_struct = get_inode_structure(inode);
    uint8_t* inode_content = (uint8_t*)kmalloc(inode_struct.size);

    // read_inode_content(&inode_struct, inode_content);
    read_inode_content(&inode_struct, 0, inode_struct.size, inode_content);

    size_t entry_pointer = 0;

    while (entry_pointer < inode_struct.size) {
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
    inode_t inode_struct = get_inode_structure(inode);
    char* inode_content = (char*)kmalloc(inode_struct.size + 1);

    read_inode_content(&inode_struct, 0, inode_struct.size, inode_content);

    inode_content[inode_struct.size] = '\0';

    term_print(inode_content);
}

}
