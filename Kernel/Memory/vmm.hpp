#pragma once

#include "Region.hpp"
#include "TranslationAllocator.hpp"
#include "pagingstructs.hpp"
#include "pmm.hpp"

#include <Hardware/CPU.hpp>
#include <Hardware/Interrupts/InterruptManager.hpp>
#include <Libkernel/Assert.hpp>
#include <Libkernel/KError.hpp>
#include <Tasking/Scheduler.hpp>

#include <Macaronlib/Common.hpp>
#include <Macaronlib/Memory.hpp>

namespace Kernel::Memory {

extern "C" void set_cr3(uint32_t page_directory_phys);
extern "C" void enable_paging();
extern "C" void flush_cr3();
extern "C" uint32_t get_cr2();

extern "C" volatile PageDir boot_page_directory;
extern "C" volatile PageTable kernel_page_table;
extern "C" volatile PageTable kernel_heap_page_table;
extern "C" volatile PageTable kernel_translation_allocator_page_table;

template <typename T>
class [[nodiscard]] PageBinder {
public:
    PageBinder(uint32_t phys, uint32_t buff_virt)
        : m_buff_virt(buff_virt)
        , m_buffered_phys_address(get_buffered_phys_address())
    {
        map_to_buffer(phys);
    }

    ~PageBinder()
    {
        map_to_buffer(m_buffered_phys_address);
    }

    T get()
    {
        return reinterpret_cast<T>(m_buff_virt);
    }

    void rebind(uint32_t phys)
    {
        map_to_buffer(phys);
    }

private:
    void map_to_buffer(uint32_t phys)
    {
        auto& pte = get_buffer_pte();
        pte.frame_adress = phys / CPU::page_size();
        pte.present = 1;
        pte.rw = 1;
        pte.user_mode = 1; // set as user for now

        flush_cr3();
    }

    uint32_t get_buffered_phys_address()
    {
        return get_buffer_pte().frame_adress * CPU::page_size();
    }

    PTEntry& get_buffer_pte()
    {
        auto page_dir = (PageDir*)((uintptr_t)&boot_page_directory + HIGHER_HALF_OFFSET);
        auto page_table = (PageTable*)(page_dir->entries[m_buff_virt / CPU::page_size() / 1024].pt_base * CPU::page_size() + HIGHER_HALF_OFFSET);
        return page_table->entries[m_buff_virt / CPU::page_size() % 1024];
    }

private:
    uint32_t m_buff_virt;
    uint32_t m_buffered_phys_address;
};

class VMM : public InterruptHandler {
public:
    static VMM& the()
    {
        static VMM the;
        return the;
    }

    uintptr_t current_translation_table() const;
    uintptr_t create_translation_table();
    void set_translation_table(uintptr_t translation_table_physical_address);

    void allocate_pages_from(size_t page, size_t pages, uint32_t flags);
    void copy_pages_cow(uintptr_t translation_table_from, size_t page, size_t pages);
    void map_pages(size_t page, size_t frame, size_t pages, uint32_t flags);
    void unmap_pages(size_t page, size_t pages);

    void allocate_memory_from(uintptr_t address, size_t bytes, uint32_t flags);
    void copy_memory_cow(uintptr_t memory_descriptor_from, uintptr_t address, size_t bytes);
    void map_memory(uintptr_t virtual_address, uintptr_t physical_address, size_t bytes, uint32_t flags);
    void unmap_memory(uintptr_t address, size_t bytes);

    //^InterruptHandler
    void handle_interrupt(Trapframe* tf) override;

    // Debug purposes.
    void inspect_page_diriectory(uint32_t page_directory_phys);
    void inspect_page_table(uint32_t page_table_phys, uint32_t page_table_index);

private:
    VMM();

    inline uint32_t create_page_table()
    {
        auto& page_table = m_tranlation_allocator.allocate_tranlation_entity<PageTable>();
        return m_tranlation_allocator.virtual_to_physical((uintptr_t)&page_table);
    }

    inline void assure_page_table(PDEntry& pde, uint32_t flags)
    {
        if (!pde._bits) {
            pde.pt_base = create_page_table() / CPU::page_size();
            pde._bits |= (flags & 0x7);
        }
    }

    inline uint32_t clone_frame(uint32_t src_frame_phys)
    {
        uint32_t dest_frame_phys = PMM::the().allocate_frame() * CPU::page_size();

        auto src_page = PageBinder<void*>(src_frame_phys, m_buffer_1);
        auto dest_page = PageBinder<void*>(dest_frame_phys, m_buffer_2);

        memcpy(dest_page.get(), src_page.get(), CPU::page_size());

        return dest_frame_phys;
    }

    inline void free_frame(uint32_t frame)
    {
        PMM::the().free_frame(frame);
    }

private:
    // virtual addresses of buffers
    uint32_t m_buffer_1;
    uint32_t m_buffer_2;

    TranslationAllocator m_tranlation_allocator {};
};

}
