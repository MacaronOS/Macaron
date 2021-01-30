#ifndef MISTIX_KERNEL_MEMORY_REGIONS_H
#define MISTIX_KERNEL_MEMORY_REGIONS_H

#include "../types.hpp"

#define HIGHER_HALF_OFFSET 0xC0000000
#define KERNEL_HEAP_SIZE 4 * 1024 * 1024

uint32_t get_kernel_start(bool phys = true);
uint32_t get_kernel_end(bool phys = true);

uint32_t get_kernel_stack_start(bool phys = true);
uint32_t get_kernel_stack_end(bool phys = true);

uint32_t get_kernel_heap_start(bool phys = true);
uint32_t get_kernel_heap_end(bool phys = true);

uint32_t get_kernel_pmm_bitmap_start(bool phys = true);

// temp page places to access in vmm
uint32_t get_pd_temp_location();
uint32_t get_pt_temp_location();

#endif // MISTIX_KERNEL_MEMORY_REGIONS_H