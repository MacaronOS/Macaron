#include "types.hpp"
#include "regions.hpp"
#include "pmm.hpp"

#include "algo/bitmap.hpp"
#include "../memory/memory.hpp"
#include "../monitor.hpp"
#include "../multiboot.hpp"

#define KB_TO_BLOCKS(x) (x * KB / BLOCK_SIZE)

bitmap_t pmmap; // contains the state of each memory block (0 - unused, 1 - used)

void pmm_init_region(uint32_t base, size_t size)
{
    for (size_t block = base / BLOCK_SIZE; block < base / BLOCK_SIZE + size / BLOCK_SIZE; block++) {
        bitmap_set_false(&pmmap, block);
    }
}

void pmm_init_available_regions(multiboot_info_t* multiboot_structure)
{
    for (
        multiboot_memory_map_t* mmap = reinterpret_cast<multiboot_memory_map_t*>(multiboot_structure->mmap_addr);
        mmap < (multiboot_memory_map_t*)(multiboot_structure->mmap_addr + multiboot_structure->mmap_length);
        mmap++
    ) {
        if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE) {
            pmm_init_region(mmap->addr, mmap->len);
        }
    }
}

void pmm_deinit_region(uint32_t base, size_t size) {
        for (size_t block = base / BLOCK_SIZE; block < base / BLOCK_SIZE + size / BLOCK_SIZE; block++) {
        bitmap_set_true(&pmmap, block);
    }
}

void* pmm_allocate_block(ShouldZeroFill s)
{
    uint32_t block = bitmap_find_first_zero(&pmmap);

    if (block == BITMAP_NULL) {
        return 0;
    }
    memset((void*)(block * BLOCK_SIZE), 0, 4096);
    bitmap_set_true(&pmmap, block); // mark this block as occupied
    return reinterpret_cast<void*>(block * BLOCK_SIZE); // return the physical locatiin of the available block
}

void pmm_free_block(void* block_address)
{
    bitmap_set_false(&pmmap, (uint32_t)block_address / BLOCK_SIZE);
}

void pmm_init(multiboot_info_t* mb_structure)
{
    const uint32_t mem_size = (mb_structure->mem_upper + 1024) * KB; // how much memory we have
    const uint32_t blocks_size = mem_size / BLOCK_SIZE; // how much blocks of memory we manage

    // initializing pmmap bitmap right after the kernel
    const uint32_t bitmap_memory_size = bitmap_init_at_location(&pmmap, blocks_size, get_kernel_pmm_bitmap_start());

    bitmap_set_all(&pmmap, 1); // for now, all blocks are inaccessible

    pmm_init_available_regions(mb_structure); // sets free memory blocks in pmmap

    // multiboot doesn't know that where kernel is, so we need to set this blocks manually
    for (
        uint32_t kernel_block_address = get_kernel_start();
        kernel_block_address < get_kernel_end() ;
        kernel_block_address += BLOCK_SIZE
    ) {
        bitmap_set_true(&pmmap, kernel_block_address / BLOCK_SIZE);
    }

    // same for the kernel stack
    for (
        uint32_t kernel_stack_block_address = get_kernel_stack_start();
        kernel_stack_block_address < get_kernel_stack_end();
        kernel_stack_block_address += BLOCK_SIZE
    ) {
        bitmap_set_true(&pmmap, kernel_stack_block_address / BLOCK_SIZE);
    }

    // and the same for the pmmap location
    // at first, caclulate the size of the bitmap (in blocks)
    uint32_t bitmap_block_size = bitmap_memory_size / BLOCK_SIZE;
    if (bitmap_memory_size % BLOCK_SIZE) {
        bitmap_block_size++;
    }

    // then set this blocks to occypied
    for (uint32_t cur_bitmap_block = 0; cur_bitmap_block < bitmap_block_size; cur_bitmap_block++) {
        bitmap_set_true(&pmmap, (uint32_t)get_kernel_pmm_bitmap_start() / BLOCK_SIZE + cur_bitmap_block);
    }

}