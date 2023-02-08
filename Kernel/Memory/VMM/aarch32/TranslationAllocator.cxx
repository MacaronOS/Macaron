#include "../TranslationAllocator.hpp"
#include "TranslationTables.hpp"

#include <Memory/Layout/Layout.hpp>
#include <Memory/Utils.hpp>

namespace Kernel::Memory {

extern "C" volatile SecondLevelTranslationTable translation_allocator_table_1;
extern "C" volatile SecondLevelTranslationTable translation_allocator_table_2;
extern "C" volatile SecondLevelTranslationTable translation_allocator_table_3;
extern "C" volatile SecondLevelTranslationTable translation_allocator_table_4;

TranslationAllocator::TranslationAllocator()
{
    size_t translation_area_start = Layout::GetLocationVirt(LayoutElement::TranslationAllocatorAreaStart);
    size_t translation_area_end = Layout::GetLocationVirt(LayoutElement::TranslationAllocatorAreaEnd);
    size_t page_count = (translation_area_end - translation_area_start) / CPU::page_size();

    m_allocated_pages = Bitmap(page_count);
    m_allocated_pages.clear();
    m_translation_area_start = translation_area_start;
}

void* TranslationAllocator::allocate_bytes(size_t bytes, size_t alignment)
{
    auto pages = bytes_to_pages(bytes);
    auto alignment_in_pages = bytes_to_pages(alignment);
    auto page_start = m_allocated_pages.occupy_sequential(pages, alignment_in_pages);
    if (page_start == BITMAP_NULL) {
        return nullptr;
    }

    auto address = m_translation_area_start + page_start * CPU::page_size();
    memset((void*)address, 0, bytes);

    return reinterpret_cast<void*>(address);
}

void TranslationAllocator::deallocate_bytes(void* address, size_t bytes)
{
    auto page_start = ((uintptr_t)address - m_translation_area_start) / CPU::page_size();
    auto pages = bytes_to_pages(bytes);

    auto cur_page = page_start;
    auto pages_left = pages;
    while (pages_left) {
        m_allocated_pages.set_false(cur_page);
        pages_left--;
        cur_page++;
    }
}

uintptr_t TranslationAllocator::physical_to_virtual(uintptr_t physical_address)
{
    return Layout::PhysToVirt(physical_address);
}

uintptr_t TranslationAllocator::virtual_to_physical(uintptr_t virtual_address)
{
    return Layout::VirtToPhys(virtual_address);
}

}