#pragma once

#include "Region.hpp"
#include "pagingstructs.hpp"
#include "pmm.hpp"

#include <Hardware/Interrupts/InterruptManager.hpp>
#include <Libkernel/Assert.hpp>
#include <Libkernel/KError.hpp>
#include <Tasking/Scheduler.hpp>

#include <Macaronlib/Common.hpp>
#include <Macaronlib/Memory.hpp>

#define PAGE_SIZE 4096

namespace Kernel::Memory {

// int paging.s
extern "C" void set_cr3(uint32_t page_directory_phys);
extern "C" void enable_paging();
extern "C" void flush_cr3();
extern "C" uint32_t get_cr2();

extern "C" uint32_t boot_page_directory;
extern "C" uint32_t boot_page_table1;
extern "C" uint32_t boot_page_table2;

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
        pte.frame_adress = phys / PAGE_SIZE;
        pte.present = 1;
        pte.rw = 1;
        pte.user_mode = 1; // set as user for now

        flush_cr3();
    }

    uint32_t get_buffered_phys_address()
    {
        return get_buffer_pte().frame_adress * PAGE_SIZE;
    }

    PTEntry& get_buffer_pte()
    {
        // at first, calculating a page table which locates buffer.
        // as buffers are in a higher half and they were initially shifted
        // by HIGHER_HALF_OFFSET=0xC0000000, we know, that the page table is one of 2 - 768 or 769

        static PageTable* page_tables[] = {
            (PageTable*)((uint32_t)&boot_page_table1 + HIGHER_HALF_OFFSET),
            (PageTable*)((uint32_t)&boot_page_table2 + HIGHER_HALF_OFFSET),
        };

        PageTable* pt = page_tables[m_buff_virt / PAGE_SIZE / 1024 - 768];

        // now, find what offset has this buffer withing the page table
        return pt->entries[m_buff_virt / PAGE_SIZE % 1024];
    }

private:
    uint32_t m_buff_virt;
    uint32_t m_buffered_phys_address;
};

class VMM : public InterruptHandler {
public:
    static VMM& the()
    {
        static VMM the {};
        return the;
    }

    uint32_t kernel_page_directory() const { return m_kernel_directory_phys; }
    uint32_t current_page_directory() const { return m_cur_page_dir_phys; }

    void set_page_directory(uint32_t page_directory_phys);

    /*
        psized - page sized
    */
    void psized_map(uint32_t pdir_phys, uint32_t page, uint32_t frame, uint32_t pages, uint32_t flags);
    void psized_unmap(uint32_t pdir_phys, uint32_t page, uint32_t pages);
    void psized_copy(uint32_t pdir_phys_to, uint32_t pdir_phys_from, uint32_t page, uint32_t pages);
    void psized_copy_allocated(uint32_t pdir_phys_to, uint32_t pdir_phys_from, uint32_t page, uint32_t pages);
    void psized_copy_allocated_as_cow(uint32_t pdir_phys_to, uint32_t pdir_phys_from, uint32_t page, uint32_t pages);
    void psized_free(uint32_t pdir_phys, uint32_t page, uint32_t pages);
    void psized_allocate_space_from(uint32_t pdir_phys, uint32_t page, uint32_t pages, uint32_t flags);

    KErrorOr<uint32_t> psized_allocate_space(uint32_t page_directory_phys, uint32_t pages, uint32_t flags);
    KErrorOr<uint32_t> psized_find_free_space(uint32_t page_directory_phys, uint32_t pages);

    inline void map(uint32_t pdir_phys, uint32_t virt_addr, uint32_t phys_addr, uint32_t size, uint32_t flags)
    {
        psized_map(pdir_phys, virt_addr / PAGE_SIZE, phys_addr / PAGE_SIZE, (size + PAGE_SIZE - 1) / PAGE_SIZE, flags);
    }

    inline void unmap(uint32_t pdir_phys, uint32_t virt_addr, uint32_t size)
    {
        psized_unmap(pdir_phys, virt_addr / PAGE_SIZE, (size + PAGE_SIZE - 1) / PAGE_SIZE);
    }

    inline void copy(uint32_t pdir_phys_to, uint32_t pdir_phys_from, uint32_t virt_addr, uint32_t size)
    {
        psized_copy(pdir_phys_to, pdir_phys_from, virt_addr / PAGE_SIZE, (size + PAGE_SIZE - 1) / PAGE_SIZE);
    }

    inline void copy_allocated(uint32_t pdir_phys_to, uint32_t pdir_phys_from, uint32_t virt_addr, uint32_t size)
    {
        return psized_copy_allocated(pdir_phys_to, pdir_phys_from, virt_addr / PAGE_SIZE, (size + PAGE_SIZE - 1) / PAGE_SIZE);
    }

    inline void copy_allocated_as_cow(uint32_t pdir_phys_to, uint32_t pdir_phys_from, uint32_t virt_addr, uint32_t size)
    {
        return psized_copy_allocated_as_cow(pdir_phys_to, pdir_phys_from, virt_addr / PAGE_SIZE, (size + PAGE_SIZE - 1) / PAGE_SIZE);
    }

    inline void free(uint32_t pdir_phys, uint32_t virt_addr, uint32_t size)
    {
        psized_free(pdir_phys, virt_addr / PAGE_SIZE, (size + PAGE_SIZE - 1) / PAGE_SIZE);
    }

    inline void allocate_space_from(uint32_t pdir_phys, uint32_t virt_addr, uint32_t size, uint32_t flags)
    {
        psized_allocate_space_from(pdir_phys, virt_addr / PAGE_SIZE, (size + PAGE_SIZE - 1) / PAGE_SIZE, flags);
    }

    inline KErrorOr<uint32_t> allocate_space(uint32_t pdir_phys, uint32_t size, uint32_t flags)
    {
        auto res = psized_allocate_space(pdir_phys, (size + PAGE_SIZE - 1) / PAGE_SIZE, flags);
        if (res) {
            return res.result() * PAGE_SIZE;
        }
        return res;
    }

    KErrorOr<uint32_t> find_free_space(uint32_t pdir_phys, uint32_t size)
    {
        auto res = psized_find_free_space(pdir_phys, (size + PAGE_SIZE - 1) / PAGE_SIZE);
        if (res) {
            return res.result() * PAGE_SIZE;
        }
        return res;
    }

    inline uint32_t create_page_directory()
    {
        auto pd_phys = PMM::the().allocate_frame() * PAGE_SIZE;
        auto pd_virt = PageBinder<PageDir*>(pd_phys, m_buffer_2);
        memset(pd_virt.get(), 0, sizeof(PageDir));

        // kernel page tables, that has been filled in Boot.s, they are used in PageBinder, so
        // they should be merged in every page directory
        pd_virt.get()->entries[768].pt_base = ((uint32_t)&boot_page_table1) / FRAME_SIZE;
        pd_virt.get()->entries[768].present = true;
        pd_virt.get()->entries[768].rw = true;
        pd_virt.get()->entries[768].user_mode = true;

        pd_virt.get()->entries[769].pt_base = ((uint32_t)&boot_page_table2) / FRAME_SIZE;
        pd_virt.get()->entries[769].present = true;
        pd_virt.get()->entries[769].rw = true;
        pd_virt.get()->entries[769].user_mode = true;

        return pd_phys;
    }

    inline void free_page_directory(uint32_t pd_phys)
    {
        set_page_directory(m_kernel_directory_phys);

        auto pd_virt = PageBinder<PageDir*>(pd_phys, m_buffer_1);

        size_t pd_index = 0;
        for (auto& pd_entry : pd_virt.get()->entries) {
            if (pd_index != 768 && pd_index != 769) {
                if (pd_entry.pt_base) {
                    auto pt_virt = PageBinder<PageTable*>(pd_entry.pt_base, m_buffer_2);
                    for (auto& pt_entry : pt_virt.get()->entries) {
                        if (pt_entry.frame_adress) {
                            PMM::the().free_frame(pt_entry.frame_adress);
                        }
                    }
                    PMM::the().free_frame(pd_entry.pt_base);
                }
            }
            pd_index++;
        }

        PMM::the().free_frame(pd_phys / FRAME_SIZE);
    }

    // interrupt handler functions:
    void handle_interrupt(Trapframe* tf) override;

    // debug purpose
    void inspect_page_diriectory(uint32_t page_directory_phys);
    void inspect_page_table(uint32_t page_table_phys, uint32_t page_table_index);

private:
    VMM();

    inline uint32_t create_page_table()
    {
        uint32_t page_table_phys = PMM::the().allocate_frame() * FRAME_SIZE;
        auto page_table_virt = PageBinder<PageTable*>(page_table_phys, m_buffer_2);
        memset(page_table_virt.get(), 0, sizeof(PageTable));
        return page_table_phys;
    }

    inline void create_ptable_if_neccesary(PDEntry& pde, uint32_t flags)
    {
        if (!pde._bits) {
            pde.pt_base = create_page_table() / PAGE_SIZE;
            pde._bits |= (flags & 0x7);
        }
    }

    inline uint32_t clone_frame(uint32_t src_frame_phys)
    {
        uint32_t dest_frame_phys = PMM::the().allocate_frame() * FRAME_SIZE;

        auto src_page = PageBinder<void*>(src_frame_phys, m_buffer_1);
        auto dest_page = PageBinder<void*>(dest_frame_phys, m_buffer_2);

        memcpy(dest_page.get(), src_page.get(), PAGE_SIZE);

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

    uint32_t m_kernel_directory_phys { (uint32_t)&boot_page_directory };
    uint32_t m_cur_page_dir_phys { (uint32_t)&boot_page_directory };
};

}
