#include "ext2.hpp"

#include "../drivers/disc/Ata.hpp"

#include "../memory/kmalloc.hpp"

#include "../assert.hpp"
#include "../memory/memory.hpp"
#include "../monitor.hpp"

#define EXT2_MAGIC 0xEF53

#define SUPERBLOCK_LOCATION 1024
#define SUPERBLOCK_SIZE 1024

#define BGDT_LOCATION (SUPERBLOCK_LOCATION + SUPERBLOCK_SIZE)

namespace kernel::fs::ext2 {

Ext2::Ext2(drivers::DiscDriver& disc_driver)
    : m_disc_driver(disc_driver)
{
}

Ext2::~Ext2()
{
    if (m_bgd_table) {
        delete[] m_bgd_table;
    }
}

bool Ext2::init()
{
    m_disc_driver.read(SUPERBLOCK_LOCATION / BYTES_PER_SECTOR, SUPERBLOCK_SIZE / BYTES_PER_SECTOR, &m_superblock);

    if (m_superblock.magic != EXT2_MAGIC) {
        return false;
    }
    m_block_size = (1024 << m_superblock.block_shift);

    m_bgd_table_size = (m_superblock.blocks_count + m_superblock.blocks_per_block_group - 1) / m_superblock.blocks_per_block_group;

    m_bgd_table = new block_group_descriptor_t[m_block_size / sizeof(block_group_descriptor_t)];

    m_disc_driver.read(BGDT_LOCATION / BYTES_PER_SECTOR, m_block_size / BYTES_PER_SECTOR, m_bgd_table);

    return true;
}

uint32_t Ext2::read(File& file, uint32_t offset, uint32_t size, uint8_t* buffer)
{
    return 0;
}

void Ext2::read_from_block_pointers_table(uint32_t bpt[], uint32_t bpt_size, void* mem)
{
    size_t read_size = 0;
    for (size_t bpt_index = 0; bpt_index < bpt_size && bpt[bpt_index] != 0; bpt_index++) {
        m_disc_driver.read(bpt[bpt_index] * 2, 2, reinterpret_cast<void*>((uint32_t)mem + read_size));
        read_size += 1024;
    }
}

void Ext2::read_inode_content(inode_t* inode, void* mem)
{
    uint32_t read_bytes = 0;

    // read from direct block pointers
    read_from_block_pointers_table(inode->direct_block_pointers, 12, (void*)((uint32_t)mem + read_bytes));
    read_bytes += 12 * 1024;

    // read from singly inderected
    if (inode->singly_inderect_block_pointer) {
        uint32_t indirect_bpt[1024 / sizeof(uint32_t)];
        m_disc_driver.read(inode->singly_inderect_block_pointer * 2, 2, &indirect_bpt);
        read_from_block_pointers_table(indirect_bpt, 1024 / sizeof(uint32_t), (void*)((uint32_t)mem + read_bytes));
    }

    read_bytes += (1024 / sizeof(uint32_t)) * 1024;

    // read from doubly inderected
    if (inode->doubly_inderect_block_pointer) {
        uint32_t doubly_inderect_bpt[1024 / sizeof(uint32_t)];
        m_disc_driver.read(inode->doubly_inderect_block_pointer * 2, 2, &doubly_inderect_bpt);

        for (size_t i = 0; i < 1024 / sizeof(uint32_t); i++) {
            uint32_t indirect_bpt[1024 / sizeof(uint32_t)];
            m_disc_driver.read(doubly_inderect_bpt[i] * 2, 2, &indirect_bpt);
            read_from_block_pointers_table(indirect_bpt, 1024 / sizeof(uint32_t), (void*)((uint32_t)mem + read_bytes));
        }

        read_bytes += (1024 / sizeof(uint32_t)) * 1024;
    }

    // read from triply inderected
    if (inode->triply_inderect_block_pointer) {
        uint32_t triply_inderect_bpt[1024 / sizeof(uint32_t)];
        m_disc_driver.read(inode->triply_inderect_block_pointer * 2, 2, &triply_inderect_bpt);

        for (size_t i = 0; i < 1024 / sizeof(uint32_t); i++) {
            uint32_t doubly_inderect_bpt[1024 / sizeof(uint32_t)];
            m_disc_driver.read(triply_inderect_bpt[i] * 2, 2, &doubly_inderect_bpt);

            for (size_t j = 0; j < 1024 / sizeof(uint32_t); j++) {
                uint32_t indirect_bpt[1024 / sizeof(uint32_t)];
                m_disc_driver.read(doubly_inderect_bpt[j] * 2, 2, &indirect_bpt);
                read_from_block_pointers_table(indirect_bpt, 1024 / sizeof(uint32_t), (void*)((uint32_t)mem + read_bytes));
            }

            read_bytes += (1024 / sizeof(uint32_t)) * 1024;
        }
    }
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
    m_disc_driver.read(inode_block * 2, 2, &inodes);

    return inodes[inode_block_index];
}

void Ext2::read_inode(uint32_t inode)
{
    inode_t inode_struct = get_inode_structure(inode);
    uint8_t* inode_content = (uint8_t*)kmalloc(inode_struct.size);

    read_inode_content(&inode_struct, inode_content);

    size_t entry_pointer = 0;

    while (entry_pointer < inode_struct.size) {
        uint32_t name_size = (((dir_entry_t*)(inode_content + entry_pointer))[0].name_len_low);
        char name[1024];
        memcpy(name, &((dir_entry_t*)(inode_content + entry_pointer))[0].name_characters, name_size);
        name[name_size] = '\0';
        term_print(name);
        term_print("\n");

        entry_pointer += ((dir_entry_t*)(inode_content + entry_pointer))[0].size;
    }
}

}