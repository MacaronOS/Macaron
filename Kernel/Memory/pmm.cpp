#include "pmm.hpp"
#include "Layout.hpp"

#include <Libkernel/Logger.hpp>

#include <Macaronlib/Memory.hpp>

namespace Kernel::Memory {

void PMM::initialize(multiboot_info_t* multiboot_info)
{
    const size_t mem_size = multiboot_info->mem_upper * KB; // how much memory we have
    const size_t blocks_size = mem_size / CPU::page_size(); // how much blocks of memory we manage

    m_pmmap = Bitmap(blocks_size);
    m_pmmap.fill();

    for (
        auto* mmap = reinterpret_cast<multiboot_memory_map_t*>(multiboot_info->mmap_addr + HIGHER_HALF_OFFSET);
        mmap < (multiboot_memory_map_t*)(multiboot_info->mmap_addr + HIGHER_HALF_OFFSET + multiboot_info->mmap_length);
        mmap++) {

        switch (mmap->type) {
        case MULTIBOOT_MEMORY_AVAILABLE:
            free_range_sized(mmap->addr, mmap->len);
            break;
        case MULTIBOOT_MEMORY_RESERVED:
            occupy_range_sized(mmap->addr, mmap->len);
            break;
        }
    }

    // Multiboot doesn't know about kernel layout, so marking these blocks manually.
    occupy_range(Layout::GetLocationPhys(LayoutElement::KernelStart), Layout::GetLocationPhys(LayoutElement::KernelInitialHeapEnd));
}

size_t PMM::allocate_frame()
{
    uint32_t frame = m_pmmap.find_first_zero();
    if (frame == BITMAP_NULL) {
        return BITMAP_NULL;
    }
    m_pmmap.set_true(frame);
    return frame;
}

void PMM::free_frame(size_t frame)
{
    m_pmmap.set_false(frame);
}

void PMM::occypy_frame(size_t frame)
{
    m_pmmap.set_true(frame);
}

void PMM::occupy_range(uintptr_t start, uintptr_t end)
{
    for (size_t frame = start / CPU::page_size(); frame < (end + CPU::page_size() - 1) / CPU::page_size(); frame++) {
        m_pmmap.set_true(frame);
    }
}

void PMM::free_range(uintptr_t start, uintptr_t end)
{
    for (size_t block = start / CPU::page_size(); block < (end + CPU::page_size() - 1) / CPU::page_size(); block++) {
        m_pmmap.set_false(block);
    }
}

size_t PMM::allocate_frames(size_t frames)
{
    return m_pmmap.occupy_sequential(frames);
}

}
