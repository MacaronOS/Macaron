#include "vmm.hpp"
#include "../assert.hpp"
#include "../monitor.hpp"
#include "../types.hpp"
#include "memory.hpp"
#include "pmm.hpp"
#include "regions.hpp"

// int paging.s
extern "C" void set_cr3(uint32_t page_directory_phys);
extern "C" void enable_paging();
extern "C" void flush_cr3();

extern "C" uint32_t boot_page_directory;
extern "C" uint32_t boot_page_table2;

template <>
VMM* Singleton<VMM>::s_t = nullptr;
template <>
bool Singleton<VMM>::s_initialized = false;

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

    page_table_virt->entries[frame_virt_addr / PAGE_SIZE % 1024].frame_adress = (uint32_t)pmm_allocate_block(ShouldZeroFill::No) / PAGE_SIZE;
    page_table_virt->entries[frame_virt_addr / PAGE_SIZE % 1024].present = true;
    page_table_virt->entries[frame_virt_addr / PAGE_SIZE % 1024].rw = true;
    page_table_virt->entries[frame_virt_addr / PAGE_SIZE % 1024].user_mode = true;

    unmap_from_buffer(m_buffer_1);
}

uint32_t VMM::clone_page_directory(uint32_t src_page_directory_phys)
{
    if (!src_page_directory_phys) {
        src_page_directory_phys = (uint32_t)&boot_page_directory - HIGHER_HALF_OFFSET;
    }

    uint32_t dest_page_dir_phys = (uint32_t)pmm_allocate_block(ShouldZeroFill::No);
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

    uint32_t page_table_phys = (uint32_t)pmm_allocate_block(ShouldZeroFill::No);
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
    uint32_t dest_page_table_phys = (uint32_t)pmm_allocate_block(ShouldZeroFill::No);

    // map page tables
    map_to_buffer(dest_page_table_phys, m_buffer_1);
    map_to_buffer(src_page_table_phys, m_buffer_2);

    page_table_t* dest_page_table_virt = (page_table_t*)m_buffer_1;
    page_table_t* src_page_table_virt = (page_table_t*)m_buffer_2;

    memset(dest_page_table_virt, 0, sizeof(page_table_t));
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

    uint32_t dest_frame_phys = (uint32_t)pmm_allocate_block(ShouldZeroFill::No);

    map_to_buffer(dest_frame_phys, m_buffer_1);
    map_to_buffer(src_frame_phys, m_buffer_2);

    memcpy((void*)m_buffer_1, (void*)m_buffer_2, PAGE_SIZE);

    map_to_buffer(buffer_1_cp, m_buffer_1);
    map_to_buffer(buffer_2_cp, m_buffer_2);

    return dest_frame_phys;
}

void VMM::map_to_buffer(uint32_t phys, uint32_t buff)
{
    uint32_t pd_pt_index = buff / PAGE_SIZE % 1024;
    page_table_t* pt = (page_table_t*)&boot_page_table2;

    pt->entries[pd_pt_index].frame_adress = phys / PAGE_SIZE;
    pt->entries[pd_pt_index].present = 1;
    pt->entries[pd_pt_index].rw = 1;
    pt->entries[pd_pt_index].user_mode = 1; // set as user for now

    flush_cr3();
}

void VMM::unmap_from_buffer(uint32_t buff)
{
    uint32_t pd_pt_index = buff / PAGE_SIZE % 1024;
    page_table_t* pt = (page_table_t*)&boot_page_table2;

    pt->entries[pd_pt_index].__bits = 0;
}

uint32_t VMM::get_buffered_phys_address(uint32_t buff)
{
    uint32_t pd_pt_index = buff / PAGE_SIZE % 1024;
    page_table_t* pt = (page_table_t*)&boot_page_table2;

    return pt->entries[pd_pt_index].frame_adress * PAGE_SIZE;
}
