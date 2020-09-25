#ifndef MISTIX_KERNEL_MEMORY_PMM_H
#define MISTIX_KERNEL_MEMORY_PMM_H

#include "../multiboot.hpp"
#include "../types.hpp"

#define BLOCK_SIZE 4096

enum class ShouldZeroFill {
    No,
    Yes
};

void pmm_init(multiboot_info_t*);
void* pmm_allocate_block(ShouldZeroFill = ShouldZeroFill::Yes);
void pmm_free_block(void*);

#endif // MISTIX_KERNEL_MEMORY_PMM_H