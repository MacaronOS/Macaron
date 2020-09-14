#include "vmm.h"
#include "pmm.h"
#include "memory.h"
#include "../types.h"

// int paging.s
extern void set_cr3(uint32_t page_directory_base_adress);
extern void enable_paging();

page_directory_t* cur_directory = 0; // keeps a pointer to the current active page directory

#define PAGE_DIRECTORY_INDEX(x) (((x) >> 22) & 0x3ff)
#define PAGE_TABLE_INDEX(x) (((x) >> 12) & 0x3ff)

#define PAGE_SIZE 4096

bool vmm_allocate_page(page_table_entry_t* pt) {
    void* frame = pmm_allocate_block();
    if (!frame) {
        return false;
    }

    pt->present = true;
    pt->frame_adress = (uint32_t)frame / PAGE_SIZE;

    return true;
}

void vmm_free_page(page_table_entry_t* pt) {
    pmm_free_block(pt->frame_adress * PAGE_SIZE);
    pt->present = false;
}

bool vmm_switch_directory(page_directory_t* dir) {
    if (dir == 0) {
        return false;
    }
    cur_directory = dir;
    set_cr3(cur_directory->entries);
}

void vmm_map_page(void* phys, void* virt) {
    uint32_t virt_index = (uint32_t)virt / PAGE_SIZE;
    page_directory_entry_t pd_entry = cur_directory->entries[virt_index / 1024];

    if (!pd_entry.present) {
        // allocating page table
        page_table_t* pt = pmm_allocate_block();
        memset(pt, 0, sizeof(page_table_t));

        pd_entry.present = 1;
        pd_entry.rw = 1;
        pd_entry.page_table_base_adress = (uint32_t)pt / PAGE_SIZE;
    }

    // create page table entry
    page_table_entry_t pt_entry;
    pt_entry.__bits = 0;
    pt_entry.present = 1;
    pt_entry.rw = 1;
    pt_entry.frame_adress = (uint32_t)phys / PAGE_SIZE;

    // set page table entry
    page_table_t* pt = pd_entry.page_table_base_adress * PAGE_SIZE;
    pt->entries[virt_index / 1024 / 1024] = pt_entry;
}

void vmm_init() {
    // allocate default page table
    page_table_t* table = pmm_allocate_block();

    for (uint32_t phyz = 0, virt = 0, i = 0; i < 1024; phyz+=PAGE_SIZE, virt += PAGE_SIZE, i++) {
        table->entries[i].__bits = 0;
        table->entries[i].present = 1;
        table->entries[i].rw = 1;
        table->entries[i].frame_adress = phyz / PAGE_SIZE;
    }

    page_directory_t* p_directory = pmm_allocate_block();
    for (size_t i = 0; i < 1024; i++) {
        p_directory->entries[i].__bits = 0;
    }

    p_directory->entries[0].__bits = 0;
    p_directory->entries[0].page_table_base_adress = (uint32_t)table / PAGE_SIZE;
    p_directory->entries[0].present = 1;
    p_directory->entries[0].rw = 1;

    cur_directory = p_directory;

    vmm_switch_directory(cur_directory);

    enable_paging();
}