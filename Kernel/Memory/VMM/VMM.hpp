#pragma once

#include "PhysicalBinder.hpp"
#include "TranslationAllocator.hpp"

#include <Hardware/CPU.hpp>
#include <Hardware/Interrupts/InterruptManager.hpp>
#include <Libkernel/Assert.hpp>
#include <Libkernel/KError.hpp>
#include <Memory/Layout.hpp>
#include <Memory/Utils.hpp>
#include <Memory/pmm.hpp>

#include <Macaronlib/Common.hpp>
#include <Macaronlib/Memory.hpp>

namespace Kernel::Memory {

enum Flags {
    Present = 1 << 0,
    Write = 1 << 1,
    User = 1 << 2,
};

class VMM : public InterruptHandler {
public:
    static VMM& the()
    {
        static VMM the;
        return the;
    }

    uintptr_t page_fault_linear_address() const;
    uintptr_t current_translation_table() const;
    uintptr_t create_translation_table();
    void set_translation_table(uintptr_t translation_table_physical_address);

    void allocate_pages_from(size_t page, size_t pages, uint32_t flags);
    void copy_pages_cow(uintptr_t translation_table_from, size_t page, size_t pages);
    void map_pages(size_t page, size_t frame, size_t pages, uint32_t flags);
    void unmap_pages(size_t page, size_t pages);

    void allocate_memory_from(uintptr_t address, size_t bytes, uint32_t flags)
    {
        allocate_pages_from(address_to_page(address), bytes_to_pages(bytes), flags);
    }

    void copy_memory_cow(uintptr_t memory_descriptor_from, uintptr_t address, size_t bytes)
    {
        copy_pages_cow(memory_descriptor_from, address_to_page(address), bytes_to_pages(bytes));
    }

    void map_memory(uintptr_t virtual_address, uintptr_t physical_address, size_t bytes, uint32_t flags)
    {
        map_pages(address_to_page(virtual_address), address_to_page(physical_address), bytes_to_pages(bytes), flags);
    }

    void unmap_memory(uintptr_t address, size_t bytes)
    {
        unmap_pages(address_to_page(address), bytes_to_pages(bytes));
    }

    //^InterruptHandler
    void handle_interrupt(Trapframe* tf) override;

private:
    VMM()
        : InterruptHandler(14)
        , m_primary_physical_binder(Layout::GetLocationVirt(LayoutElement::PagingBuffer1))
        , m_secondary_physical_binder(Layout::GetLocationVirt(LayoutElement::PagingBuffer2))
    {
    }

    uint32_t clone_frame_of_address(uint32_t physical_address)
    {
        m_primary_physical_binder.bind_physical_page(physical_address);
        auto virtual_address = m_primary_physical_binder.get();

        uint32_t new_physical_address = PMM::the().allocate_frame() * CPU::page_size();
        m_secondary_physical_binder.bind_physical_page(new_physical_address);
        auto new_virtual_address = m_secondary_physical_binder.get();

        memcpy(new_virtual_address, virtual_address, CPU::page_size());

        m_primary_physical_binder.unbind_physical_page();
        m_secondary_physical_binder.unbind_physical_page();

        return new_physical_address;
    }

private:
    PhysicalBinder m_primary_physical_binder;
    PhysicalBinder m_secondary_physical_binder;
    TranslationAllocator m_tranlation_allocator {};
};

}
