#include "../PhysicalBinder.hpp"
#include "TranslationTables.hpp"

#include <Hardware/CPU.hpp>
#include <Libkernel/Logger.hpp>
#include <Memory/Layout.hpp>

namespace Kernel::Memory {

extern "C" volatile PageTable translation_allocator_page_table;

PhysicalBinder::PhysicalBinder(uintptr_t virtual_base_address)
    : m_virtual_base_address(virtual_base_address)
{
    auto page_table_entry = virtual_base_address / CPU::page_size() % 1024;
    auto& page_table = *reinterpret_cast<PageTable*>(Layout::PhysToVirt((uintptr_t)&translation_allocator_page_table));
    m_translation_entry = &page_table.entries[page_table_entry];
}

void PhysicalBinder::bind_physical_page(uintptr_t physical_page_address)
{
    auto translation_entry = reinterpret_cast<volatile PTEntry*>(m_translation_entry);

    translation_entry->present = true;
    translation_entry->rw = true;
    translation_entry->frame_adress = physical_page_address / CPU::page_size();

    CPU::flush_tlb(m_virtual_base_address, 1);
}

void PhysicalBinder::unbind_physical_page()
{
    auto translation_entry = reinterpret_cast<volatile PTEntry*>(m_translation_entry);
    translation_entry->_bits = 0;

    CPU::flush_tlb(m_virtual_base_address, 1);
}

}