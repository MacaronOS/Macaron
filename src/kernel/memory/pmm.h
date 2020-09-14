#ifndef MISTIX_KERNEL_MEMORY_PMM_H
#define MISTIX_KERNEL_MEMORY_PMM_H

#include "../types.h"
#include "../multiboot.h"

#define BLOCK_SIZE 4096

void pmm_init(multiboot_info_t*);
void* pmm_allocate_block();
void pmm_free_block(void*);

#endif // MISTIX_KERNEL_MEMORY_PMM_H