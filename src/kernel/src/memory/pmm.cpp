#include "pmm.hpp"
#include "memory.hpp"
#include "Layout.hpp"

#include <types.hpp>
#include <multiboot.hpp>

#include <algo/Bitmap.hpp>
#include <algo/Singleton.hpp>

namespace kernel::memory {

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
    m_pmmap = kernel::Bitmap::wrap(Layout::GetLocationVirt(LayoutElement::PMMBitmapStart), blocks_size);
    m_pmmap.fill();

    for (
        multiboot_memory_map_t* mmap = reinterpret_cast<multiboot_memory_map_t*>(m_multiboot_info->mmap_addr + HIGHER_HALF_OFFSET);
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

}
