#include "Ext2FileSystem.hpp"
#include "Ext2Inode.hpp"
#include <Drivers/Disk/Ata.hpp>
#include <Macaronlib/Bitmap.hpp>
#include <Libkernel/Logger.hpp>

namespace Kernel::FileSystem::Ext2 {

void Ext2FileSystem::init()
{
    constexpr auto ext2_magic = 0xEF53;

    m_block_size = 1024;
    read_block(1, &m_superblock);
    if (m_superblock.magic != ext2_magic) {
        return;
    }

    m_block_size = (1024 << m_superblock.block_shift);
    m_bgd_table_size = (m_superblock.blocks_count + m_superblock.blocks_per_block_group - 1) / m_superblock.blocks_per_block_group;

    m_block_buffer = Vector<char>(m_block_size);
    m_own_block_buffer = Vector<char>(m_block_size);

    auto bgd_table_buffer = malloc(m_block_size);
    read_block(2, bgd_table_buffer);
    m_bgd_table = (block_group_descriptor_t*)bgd_table_buffer;

    m_root = get_inode(2);
}

Inode* Ext2FileSystem::allocate_inode()
{
    return new Ext2Inode(occypy_inode(), *this);
}

void Ext2FileSystem::write_inode(Inode& inode)
{
    auto ext2_inode = static_cast<Ext2Inode&>(inode);

    uint32_t block_group_index = (ext2_inode.inode() - 1) / m_superblock.inodes_per_block_group;
    uint32_t inode_table_block = m_bgd_table[block_group_index].inode_table_addr;
    uint32_t inode_table_index = (ext2_inode.inode() - 1) % m_superblock.inodes_per_block_group;

    // block, where inode structure is stored
    uint32_t inode_block = inode_table_block + (inode_table_index * sizeof(inode_t)) / m_block_size;

    // (index) position of inode structure inside inode block
    uint32_t inode_block_index = inode_table_index % (m_block_size / sizeof(inode_t));

    // copying block, where inode structure is stored
    read_block(inode_block, own_block_buffer());
    auto inodes = (inode_t*)own_block_buffer();

    // saving new inode structure
    inodes[inode_block_index] = ext2_inode.m_raw_inode;
    write_block(inode_block, inodes);
}

size_t Ext2FileSystem::block_size()
{
    return m_block_size;
}

uint32_t Ext2FileSystem::allocate_block()
{
    for (size_t i = 0; i < m_bgd_table_size; i++) {
        if (m_bgd_table[i].unallocated_block_count) {
            // read bitmap
            read_block(m_bgd_table[i].block_bitmap_addr, own_block_buffer());

            // find first free block in bitmap
            Bitmap bit = Bitmap::wrap((uint32_t)own_block_buffer(), m_superblock.blocks_per_block_group);
            const uint32_t free_block_in_bitmap = bit.find_first_zero();

            // occupy block
            const uint32_t free_block_in_ext2 = i * m_superblock.blocks_per_block_group + free_block_in_bitmap + 1;
            bit.set_true(free_block_in_bitmap);
            write_block(m_bgd_table[i].block_bitmap_addr, own_block_buffer());

            return free_block_in_ext2;
        }
    }

    return 0;
}

bool Ext2FileSystem::read_block(size_t block, void* buffer)
{
    return m_disk_driver.read(
        block * block_size() / BYTES_PER_SECTOR,
        block_size() / BYTES_PER_SECTOR,
        buffer);
}

bool Ext2FileSystem::write_block(size_t block, void* buffer)
{
    return m_disk_driver.write(
        block * block_size() / BYTES_PER_SECTOR,
        block_size() / BYTES_PER_SECTOR,
        buffer);
}

Ext2Inode* Ext2FileSystem::get_inode(size_t inode)
{
    auto inode_object = new Ext2Inode(inode, *this);

    uint32_t block_group_index = (inode - 1) / m_superblock.inodes_per_block_group;
    uint32_t inode_table_block = m_bgd_table[block_group_index].inode_table_addr;
    uint32_t inode_table_index = (inode - 1) % m_superblock.inodes_per_block_group;

    // block, where inode structure is stored
    uint32_t inode_block = inode_table_block + (inode_table_index * sizeof(inode_t)) / m_block_size;

    // (index) position of inode structure inside inode block
    uint32_t inode_block_index = inode_table_index % (m_block_size / sizeof(inode_t));

    read_block(inode_block, own_block_buffer());
    auto inodes = (inode_t*)own_block_buffer();

    inode_object->m_raw_inode = inodes[inode_block_index];

    return inode_object;
}

uint32_t Ext2FileSystem::occypy_inode(uint32_t preferd_block_group)
{
    for (size_t i = preferd_block_group; i < m_bgd_table_size; i++) {
        if (m_bgd_table[i].unallocated_inodes_count) {
            // read bitmap
            read_block(m_bgd_table[i].inode_bitmap_addr, own_block_buffer());

            // find first free inode in bitmap
            Bitmap bit = Bitmap::wrap((uint32_t)own_block_buffer(), m_superblock.inodes_per_block_group);
            const uint32_t free_inode = bit.find_first_zero();

            // occupy inode
            bit.set_true(free_inode);
            write_block(m_bgd_table[i].inode_bitmap_addr, own_block_buffer());

            return i * m_superblock.inodes_per_block_group + free_inode + 1; // inodes starts from 1
        }
    }

    return 0;
}

}