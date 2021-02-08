#pragma once
#include "Layout.hpp"
#include "pagingstructs.hpp"

#include <assert.hpp>
#include <types.hpp>

#include <algo/Singleton.hpp>
#include <hardware/InterruptManager.hpp>
#include <multitasking/TaskManager.hpp>

#define PAGE_SIZE 4096

namespace kernel::memory {

class VMM : public Singleton<VMM>, InterruptHandler {
public:
    VMM();

    uint32_t kernel_page_directory() const { return m_kernel_directory_phys; }

    void set_page_directory(uint32_t page_directory_phys);

    // allocates an actual frame then maps it to the frame_virt_addr
    void create_frame(uint32_t page_directory_phys, uint32_t frame_virt_addr);

    // clones an entire page directory. also makes a copy of each frame
    uint32_t clone_page_directory(uint32_t src_page_directory_phys = 0);

    // interrupt handler functions:
    void handle_interrupt(trapframe_t* tf) override;

private:
    uint32_t create_page_table();

    // clone helping functions
    uint32_t clone_page_table(uint32_t src_page_table_phys);
    uint32_t clone_frame(uint32_t src_frame_phys);

    // buffers functions
    page_table_entry_t& get_buffer_pte(uint32_t buff_virt);
    void map_to_buffer(uint32_t phys, uint32_t buf);
    void unmap_from_buffer(uint32_t buf);
    uint32_t get_buffered_phys_address(uint32_t buff);

    // interrupt handler functions:

private:
    // virtual addresses of buffers
    uint32_t m_buffer_1;
    uint32_t m_buffer_2;

    uint32_t m_kernel_directory_phys { clone_page_directory(0) }; // temp decision
    uint32_t cur_directory_phys;
};

}
