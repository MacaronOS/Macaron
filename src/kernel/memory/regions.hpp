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

#endif // MISTIX_KERNEL_MEMORY_REGIONS_H