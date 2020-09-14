#ifndef MISTIX_KERNEL_MEMORY_VMM_H
#define MISTIX_KERNEL_MEMORY_VMM_H

#include "../types.h"

typedef union {
    struct {
        uint32_t present: 1; 
        uint32_t rw: 1;
        uint32_t user_mode: 1;
        uint32_t reserved_1: 2;
        uint32_t access: 1;
        uint32_t dirty: 1;
        uint32_t reserved_2: 2;
        uint32_t available: 3;
        uint32_t frame_adress: 20;
    };
    uint32_t __bits;
} page_table_entry_t;

typedef struct {
    page_table_entry_t entries[1024];
} page_table_t;

typedef union {
    struct {
        uint32_t present: 1; 
        uint32_t rw: 1;
        uint32_t user_mode: 1;
        uint32_t write_through: 1;
        uint32_t cache: 1; // if set, page table will be cached
        uint32_t access: 1;
        uint32_t reserved: 1;
        uint32_t page_size: 1; // 0 - 4kb, 1 - 4mb
        uint32_t global_page: 1; // ignored
        uint32_t available: 3;
        uint32_t page_table_base_adress: 20;
    };
    uint32_t __bits;
} page_directory_entry_t;

typedef struct {
    page_directory_entry_t entries[1024];
} page_directory_t;

bool vmm_allocate_page(page_table_entry_t*);
void vmm_free_page(page_table_entry_t*);
void vmm_init();

#endif // MISTIX_KERNEL_MEMORY_VMM_H