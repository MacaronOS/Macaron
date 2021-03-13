#pragma once
#include "Layout.hpp"
#include "pagingstructs.hpp"

#include <assert.hpp>
#include <types.hpp>

#include <algo/Singleton.hpp>
#include <errors/KError.hpp>
#include <hardware/InterruptManager.hpp>
#include <multitasking/TaskManager.hpp>

#define PAGE_SIZE 4096

namespace kernel::memory {

// int paging.s
extern "C" void set_cr3(uint32_t page_directory_phys);
extern "C" void enable_paging();
extern "C" void flush_cr3();
extern "C" uint32_t get_cr2();

extern "C" uint32_t boot_page_directory;
extern "C" uint32_t boot_page_table1;
extern "C" uint32_t boot_page_table2;

class VMM : public Singleton<VMM>, InterruptHandler {
public:
    VMM();

    uint32_t kernel_page_directory() const { return m_kernel_directory_phys; }
    uint32_t current_page_directory() const { return m_cur_page_dir_phys; }

    void set_page_directory(uint32_t page_directory_phys);

    // allocates an actual frame then maps it to the frame_virt_addr
    void create_frame(uint32_t page_directory_phys, uint32_t frame_virt_addr);

    // maps virtual address to physical addres
    void map_virt_to_phys(uint32_t page_dir_phys, uint32_t virt_addr, uint32_t phys_addr, uint32_t size);

    // maps page to frame
    void map_page_to_frame(uint32_t page_dir_phys, uint32_t page, uint32_t frame);

    // removes all entries in all page tables except the kernel ones
    // also clears corresponding frames
    void clear_user_directory_pages(uint32_t src_page_directory_phys);

    // clones an entire page directory. also makes a copy of each frame
    uint32_t clone_page_directory(uint32_t src_page_directory_phys = 0);

    // allocates virtual space, returns adress
    KErrorOr<uint32_t> allocate_space(uint32_t page_directory_phys, uint32_t size);

    // find free spcae in page tables space (no mapping will happen)
    KErrorOr<uint32_t> find_free_space(uint32_t page_directory_phys, uint32_t size);

    // interrupt handler functions:
    void handle_interrupt(trapframe_t* tf) override;

    // debug purpose
    void inspect_page_diriectory(uint32_t page_directory_phys);
    void inspect_page_table(uint32_t page_table_phys, uint32_t page_table_index);

private:
    uint32_t create_page_table();

    // clone helping functions
    uint32_t clone_page_table(uint32_t src_page_table_phys);
    uint32_t clone_frame(uint32_t src_frame_phys);

    // unmaping helping functions
    void clear_user_table_pages(uint32_t src_page_table_phys);

private:
    // virtual addresses of buffers
    uint32_t m_buffer_1;
    uint32_t m_buffer_2;

    uint32_t m_kernel_directory_phys { clone_page_directory(0) }; // temp decision
    uint32_t m_cur_page_dir_phys;
};

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

    page_table_entry_t& get_buffer_pte()
    {
        // at first, calculating a page table which locates buffer.
        // as buffers are in a higher half and they were initialy shifted
        // by HIGHER_HALF_OFFSET=0xC0000000, we know, that the page table is one of 2 - 768 or 769

        static page_table_t* page_tables[] = {
            (page_table_t*)&boot_page_table1,
            (page_table_t*)&boot_page_table2,
        };

        page_table_t* pt = page_tables[m_buff_virt / PAGE_SIZE / 1024 - 768];

        // now, find what offset has this buffer withing the page table
        return pt->entries[m_buff_virt / PAGE_SIZE % 1024];
    }

private:
    uint32_t m_buff_virt;
    uint32_t m_buffered_phys_address;
};

}
