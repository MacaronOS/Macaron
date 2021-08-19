#include "pmm.hpp"
#include "Layout.hpp"

#include <Multiboot.hpp>

#include <Wisterialib/Bitmap.hpp>
#include <Wisterialib/Singleton.hpp>
#include <Wisterialib/common.hpp>
#include <Wisterialib/memory.hpp>

#include <Libkernel/Logger.hpp>

namespace Kernel::Memory {

template <>
PMM* Singleton<PMM>::s_t = nullptr;
template <>
bool Singleton<PMM>::s_initialized = false;

PMM::PMM(multiboot_info_t* multiboot_info)
    : m_multiboot_info(multiboot_info)
{
    const uint32_t mem_size = (m_multiboot_info->mem_upper + 1024) * KB; // how much memory we have
    const uint32_t blocks_size = mem_size / FRAME_SIZE; // how much blocks of memory we manage

    // initializing pmmap bitmap right after the kernel
    m_pmmap = Bitmap::wrap(Layout::GetLocationVirt(LayoutElement::PMMBitmapStart), blocks_size);
    m_pmmap.fill();

    for (
        auto* mmap = reinterpret_cast<multiboot_memory_map_t*>(m_multiboot_info->mmap_addr + HIGHER_HALF_OFFSET);
        mmap < (multiboot_memory_map_t*)(m_multiboot_info->mmap_addr + HIGHER_HALF_OFFSET + m_multiboot_info->mmap_length);
        mmap++) {
        if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE) {
            free_range(mmap->addr, mmap->addr + mmap->len - 1);
        }
    }

    // multiboot doesn't know that where kernel is, so we need to set this blocks manually
    occupy_range(Layout::GetLocationPhys(LayoutElement::KernelStart), Layout::GetLocationPhys(LayoutElement::KernelEnd));

    // and the same for the pmmap location
    occupy_range(Layout::GetLocationPhys(LayoutElement::PMMBitmapStart), Layout::GetLocationPhys(LayoutElement::PMMBitmapStart) + m_pmmap.memory_size() - 1);

    auto pmmap_end = (Layout::GetLocationVirt(LayoutElement::PMMBitmapStart) + m_pmmap.memory_size() + FRAME_SIZE - 1) / FRAME_SIZE * FRAME_SIZE;
    Layout::SetLocationVirt(LayoutElement::PMMBitmapEnd, pmmap_end);
}

uint32_t PMM::allocate_frame()
{
    uint32_t frame = m_pmmap.find_first_zero();
    if (frame == BITMAP_NULL) {
        return BITMAP_NULL;
    }
    m_pmmap.set_true(frame);
    return frame;
}

void PMM::free_frame(uint32_t frame)
{
    m_pmmap.set_false(frame);
}

void PMM::occypy_frame(uint32_t frame)
{
    m_pmmap.set_true(frame);
}

void PMM::occupy_range(uint32_t left, size_t right)
{
    for (size_t frame = left / FRAME_SIZE; frame <= (right + FRAME_SIZE - 1) / FRAME_SIZE; frame++) {
        m_pmmap.set_true(frame);
    }
}

void PMM::free_range(uint32_t left, size_t right)
{
    for (size_t block = left / FRAME_SIZE; block <= (right + FRAME_SIZE - 1) / FRAME_SIZE; block++) {
        m_pmmap.set_false(block);
    }
}

uint32_t PMM::allocate_frames(uint32_t frames)
{
    for (size_t start_frame = 0; start_frame < m_pmmap.size(); start_frame++) {

        uint32_t remain_frames = frames;
        size_t frame = start_frame;

        for (; frame < m_pmmap.size() && !m_pmmap[frame] && remain_frames; frame++, remain_frames--) { }

        if (!remain_frames) {
            occupy_range(start_frame * FRAME_SIZE, (start_frame + frames - 1) * FRAME_SIZE);
            return start_frame;
        }

        start_frame = frame;
    }
    return 0;
}

}
