#include "pmm.hpp"
#include "Layout.hpp"

#include <Libkernel/Logger.hpp>
#include <Multiboot.hpp>

#include <Macaronlib/Bitmap.hpp>
#include <Macaronlib/Common.hpp>
#include <Macaronlib/Memory.hpp>

namespace Kernel::Memory {

void PMM::initialize(multiboot_info_t* multiboot_info)
{
    const size_t mem_size = multiboot_info->mem_upper * KB; // how much memory we have
    const size_t blocks_size = mem_size / FRAME_SIZE; // how much blocks of memory we manage

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
    occupy_range(Layout::GetLocationPhys(LayoutElement::KernelStart), Layout::GetLocationPhys(LayoutElement::PagingBuffer2));
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
    for (size_t frame = start / FRAME_SIZE; frame < (end + FRAME_SIZE - 1) / FRAME_SIZE; frame++) {
        m_pmmap.set_true(frame);
    }
}

void PMM::free_range(uintptr_t start, uintptr_t end)
{
    for (size_t block = start / FRAME_SIZE; block < (end + FRAME_SIZE - 1) / FRAME_SIZE; block++) {
        m_pmmap.set_false(block);
    }
}

size_t PMM::allocate_frames(size_t frames)
{
    for (size_t start_frame = 0; start_frame < m_pmmap.size(); start_frame++) {

        size_t remain_frames = frames;
        size_t frame = start_frame;

        for (; frame < m_pmmap.size() && !m_pmmap[frame] && remain_frames; frame++, remain_frames--) { }

        if (!remain_frames) {
            occupy_range_sized(start_frame * FRAME_SIZE, frames * FRAME_SIZE);
            return start_frame;
        }

        start_frame = frame;
    }
    return 0;
}

}
