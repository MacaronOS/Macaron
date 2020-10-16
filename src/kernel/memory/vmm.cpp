#include "vmm.hpp"
#include "../assert.hpp"
#include "../monitor.hpp"
#include "../types.hpp"
#include "regions.hpp"
#include "memory.hpp"
#include "pmm.hpp"

// int paging.s
extern "C" void set_cr3(void* page_directory_base_adress);
extern "C" void enable_paging();

page_directory_t* cur_directory = 0; // keeps a pointer to the current active page directory

#define PAGE_DIRECTORY_INDEX(x) (((x) >> 22) & 0x3ff)
#define PAGE_TABLE_INDEX(x) (((x) >> 12) & 0x3ff)

#define PAGE_SIZE 4096

bool vmm_allocate_page(page_table_entry_t* pt)
{
    uint32_t frame = (uint32_t)pmm_allocate_block();
    if (!frame) {
        return false;
    }

    pt->present = true;
    pt->frame_adress = (uint32_t)frame / PAGE_SIZE;

    return true;
}

void vmm_free_page(page_table_entry_t* pt)
{
    pmm_free_block((void*)(pt->frame_adress * PAGE_SIZE));
    pt->present = false;
}

bool vmm_switch_directory(page_directory_t* dir)
{
    if (dir == 0) {
        return false;
    }
    cur_directory = dir;
    set_cr3(cur_directory->entries);

    return true;
}

void vmm_map_page(uint32_t phys, uint32_t virt)
{
    uint32_t virt_index = virt / PAGE_SIZE;
    page_directory_entry_t* pd_entry = &cur_directory->entries[virt_index / 1024];

    if (!pd_entry->present) {
        // allocating page table
        page_table_t* pt = reinterpret_cast<page_table_t*>(pmm_allocate_block(ShouldZeroFill::Yes, false));
        // memset(pt, 0, sizeof(page_table_t));

        pd_entry->present = 1;
        pd_entry->rw = 1;
        pd_entry->page_table_base_adress = ((uint32_t)pt - HIGHER_HALF_OFFSET) / PAGE_SIZE;
    }

    page_table_t* pt = reinterpret_cast<page_table_t*>(pd_entry->page_table_base_adress  * PAGE_SIZE + HIGHER_HALF_OFFSET);
    page_table_entry_t* pt_entry = &pt->entries[virt_index % 1024];

    pt_entry->__bits = 0;
    pt_entry->present = 1;
    pt_entry->rw = 1;
    pt_entry->frame_adress = (uint32_t)phys / PAGE_SIZE;

}

void vmm_init()
{
    // create kernel page directory
    page_directory_t* p_directory = reinterpret_cast<page_directory_t*>(pmm_allocate_block(ShouldZeroFill::Yes, false));
    
    for (size_t i = 0; i < 1024; i++) {
        p_directory->entries[i].present = 0;
    }

    cur_directory = p_directory;

    // map 8 mb
    for (uint32_t phys = 0, virt = HIGHER_HALF_OFFSET; phys < 8 * 1024 * 1024; phys += PAGE_SIZE, virt += PAGE_SIZE) {
        vmm_map_page(phys, virt);
    }
    

    vmm_switch_directory(cur_directory);
    enable_paging();
}