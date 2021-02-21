#include "vmm.hpp"
#include "Layout.hpp"
#include "memory.hpp"
#include "pagingstructs.hpp"
#include "pmm.hpp"

#include <assert.hpp>
#include <monitor.hpp>
#include <types.hpp>
#include <Logger.hpp>

#include <multitasking/TaskManager.hpp>

// int paging.s
extern "C" void set_cr3(uint32_t page_directory_phys);
extern "C" void enable_paging();
extern "C" void flush_cr3();
extern "C" uint32_t get_cr2();

extern "C" uint32_t boot_page_directory;
extern "C" uint32_t boot_page_table1;
extern "C" uint32_t boot_page_table2;

namespace kernel::memory {

using namespace Logger;

template <>
VMM* Singleton<VMM>::s_t = nullptr;
template <>
bool Singleton<VMM>::s_initialized = false;

VMM::VMM()
    : InterruptHandler(14)
    , m_buffer_1(Layout::GetLocationVirt(LayoutElement::PagingBuffer1))
    , m_buffer_2(Layout::GetLocationVirt(LayoutElement::PagingBuffer2))
{
    set_page_directory(m_kernel_directory_phys);
}

void VMM::set_page_directory(uint32_t page_directory_phys)
{
    set_cr3(page_directory_phys);
}

void VMM::create_frame(uint32_t page_directory_phys, uint32_t frame_virt_addr)
{
    map_to_buffer(page_directory_phys, m_buffer_1);

    page_directory_t* page_directory_virt = (page_directory_t*)m_buffer_1;

    if (!page_directory_virt->entries[frame_virt_addr / PAGE_SIZE / 1024].__bits) {
        page_directory_virt->entries[frame_virt_addr / PAGE_SIZE / 1024].page_table_base_adress = create_page_table() / PAGE_SIZE;
        page_directory_virt->entries[frame_virt_addr / PAGE_SIZE / 1024].present = true;
        page_directory_virt->entries[frame_virt_addr / PAGE_SIZE / 1024].rw = true;
        page_directory_virt->entries[frame_virt_addr / PAGE_SIZE / 1024].user_mode = true;
    }

    uint32_t page_table_phys = page_directory_virt->entries[frame_virt_addr / PAGE_SIZE / 1024].page_table_base_adress * PAGE_SIZE;
    map_to_buffer(page_table_phys, m_buffer_1);
    page_table_t* page_table_virt = (page_table_t*)m_buffer_1;

    if (page_table_virt->entries[frame_virt_addr / PAGE_SIZE % 1024].__bits) {
        return;
    }

    page_table_virt->entries[frame_virt_addr / PAGE_SIZE % 1024].frame_adress = PMM::the().allocate_frame();
    page_table_virt->entries[frame_virt_addr / PAGE_SIZE % 1024].present = true;
    page_table_virt->entries[frame_virt_addr / PAGE_SIZE % 1024].rw = true;
    page_table_virt->entries[frame_virt_addr / PAGE_SIZE % 1024].user_mode = true;

    flush_cr3();
}

uint32_t VMM::clone_page_directory(uint32_t src_page_directory_phys)
{
    if (!src_page_directory_phys) {
        src_page_directory_phys = (uint32_t)&boot_page_directory - HIGHER_HALF_OFFSET;
    }

    uint32_t dest_page_dir_phys = PMM::the().allocate_frame() * FRAME_SIZE;
    map_to_buffer(dest_page_dir_phys, m_buffer_1);
    map_to_buffer(src_page_directory_phys, m_buffer_2);

    page_directory_t* dest_page_dir_virt = (page_directory_t*)m_buffer_1;
    page_directory_t* src_page_dir_virt = (page_directory_t*)m_buffer_2;

    memset(dest_page_dir_virt, 0, sizeof(page_directory_t));

    // map kernel tables to the same location
    dest_page_dir_virt->entries[768] = src_page_dir_virt->entries[768];
    dest_page_dir_virt->entries[769] = src_page_dir_virt->entries[769];

    for (size_t i = 0; i < 768; i++) {
        if (src_page_dir_virt->entries[i].__bits) {
            dest_page_dir_virt->entries[i] = src_page_dir_virt->entries[i];
            dest_page_dir_virt->entries[i].page_table_base_adress = clone_page_table(src_page_dir_virt->entries[i].page_table_base_adress * PAGE_SIZE) / PAGE_SIZE;
        }
    }

    unmap_from_buffer(m_buffer_1);
    unmap_from_buffer(m_buffer_2);

    return dest_page_dir_phys;
}

uint32_t VMM::create_page_table()
{
    uint32_t buffer_1_cp = get_buffered_phys_address(m_buffer_1);

    uint32_t page_table_phys = PMM::the().allocate_frame() * FRAME_SIZE;
    map_to_buffer(page_table_phys, m_buffer_1);
    page_table_t* page_table_virt = (page_table_t*)m_buffer_1;
    memset(page_table_virt, 0, sizeof(page_table_t));

    map_to_buffer(buffer_1_cp, m_buffer_1);

    return page_table_phys;
}

uint32_t VMM::clone_page_table(uint32_t src_page_table_phys)
{
    uint32_t buffer_1_cp = get_buffered_phys_address(m_buffer_1);
    uint32_t buffer_2_cp = get_buffered_phys_address(m_buffer_2);

    // allocate space for the new page table
    uint32_t dest_page_table_phys = PMM::the().allocate_frame() * FRAME_SIZE;

    // map page tables
    map_to_buffer(dest_page_table_phys, m_buffer_1);
    map_to_buffer(src_page_table_phys, m_buffer_2);

    page_table_t* dest_page_table_virt = (page_table_t*)m_buffer_1;
    page_table_t* src_page_table_virt = (page_table_t*)m_buffer_2;

    memset(dest_page_table_virt, 0, sizeof(page_table_t));
    flush_cr3();
    for (size_t i = 0; i < 1024; i++) {
        if (src_page_table_virt->entries[i].__bits) {
            dest_page_table_virt->entries[i] = src_page_table_virt->entries[i];
            dest_page_table_virt->entries[i].frame_adress = clone_frame(src_page_table_virt->entries[i].frame_adress * PAGE_SIZE) / PAGE_SIZE;
        }
    }

    map_to_buffer(buffer_1_cp, m_buffer_1);
    map_to_buffer(buffer_2_cp, m_buffer_2);

    return dest_page_table_phys;
}

uint32_t VMM::clone_frame(uint32_t src_frame_phys)
{
    uint32_t buffer_1_cp = get_buffered_phys_address(m_buffer_1);
    uint32_t buffer_2_cp = get_buffered_phys_address(m_buffer_2);

    uint32_t dest_frame_phys = PMM::the().allocate_frame() * FRAME_SIZE;

    map_to_buffer(dest_frame_phys, m_buffer_1);
    map_to_buffer(src_frame_phys, m_buffer_2);

    memcpy((void*)m_buffer_1, (void*)m_buffer_2, PAGE_SIZE);

    map_to_buffer(buffer_1_cp, m_buffer_1);
    map_to_buffer(buffer_2_cp, m_buffer_2);

    return dest_frame_phys;
}

void VMM::map_to_buffer(uint32_t phys, uint32_t buff_virt)
{
    auto& pte = get_buffer_pte(buff_virt);
    pte.frame_adress = phys / FRAME_SIZE;
    pte.present = 1;
    pte.rw = 1;
    pte.user_mode = 1; // set as user for now

    flush_cr3();
}

void VMM::unmap_from_buffer(uint32_t buff_virt)
{
    get_buffer_pte(buff_virt).__bits = 0;
    flush_cr3();
}

uint32_t VMM::get_buffered_phys_address(uint32_t buff_virt)
{
    return get_buffer_pte(buff_virt).frame_adress * PAGE_SIZE;
}

page_table_entry_t& VMM::get_buffer_pte(uint32_t buff_virt)
{
    // at first, calculating a page table which locates buffer.
    // as buffers are in a higher half and they were initialy shifted
    // by HIGHER_HALF_OFFSET=0xC0000000, we know, that the page table is one of 2 - 768 or 769

    static page_table_t* page_tables[] = {
        (page_table_t*)&boot_page_table1,
        (page_table_t*)&boot_page_table2,
    };

    page_table_t* pt = page_tables[buff_virt / PAGE_SIZE / 1024 - 768];

    // now, find what offset has this buffer withing the page table
    return pt->entries[buff_virt / PAGE_SIZE % 1024];
}

void VMM::handle_interrupt(trapframe_t* tf)
{
    term_print("\nPage fault! Info:\n");

    term_print("Virtual address: ");
    term_printd(get_cr2());

    static PageFaultFlag flags[] = {
        PageFaultFlag::Present,
        PageFaultFlag::Write,
        PageFaultFlag::User,
        PageFaultFlag::ReservedWrite,
        PageFaultFlag::InstructionFetch,
    };

    static char* descr[] = {
        "Present",
        "Write",
        "User",
        "ReservedWrite",
        "InstructionFetch",
    };

    term_print("\nFlags: ");

    for (auto flag : flags) {
        if (tf->err_code & (1 << static_cast<uint32_t>(flag))) {
            term_print(descr[static_cast<uint32_t>(flag)]);
            term_print(" ");
        }
    }

    term_print("\nPID:");
    term_printd(multitasking::TaskManager::the().cur_thread()->process->id);
    term_print("\n");

    STOP();
}

void VMM::inspect_page_diriectory(uint32_t page_directory_phys)
{

    map_to_buffer(page_directory_phys, m_buffer_2);
    page_directory_t* page_dir_virt = (page_directory_t*)m_buffer_2;

    for (size_t i = 0; i < 1024; i++) {
        if (i == 768 || i == 769) {
            continue; // skip kernel page tables
        }
        if (page_dir_virt->entries[i].__bits) {
            inspect_page_table(page_dir_virt->entries[i].page_table_base_adress * 4096, i);
        }
    }
}

void VMM::inspect_page_table(uint32_t page_table_phys, uint32_t page_table_index)
{
    map_to_buffer(page_table_phys, m_buffer_1);
    page_table_t* page_table_virt = (page_table_t*)m_buffer_1;

    for (size_t i = 0; i < 1024; i++) {
        if (page_table_virt->entries[i].__bits) {
            Log() << "Page: " << (page_table_index * 1024 + i) * 4096 << " , Frame: " << page_table_virt->entries[i].frame_adress * 4096 << "\n";
        }
    }
}

}