#ifndef MISTIX_KERNEL_MEMORY_REGIONS_H
#define MISTIX_KERNEL_MEMORY_REGIONS_H

#include "../types.hpp"

#define KERNEL_STACK_SIZE 16384
#define KERNEL_HEAP_SIZE  4 * 1024 * 1024

uint32_t get_kernel_stack_start();
uint32_t get_kernel_stack_end();

uint32_t get_kernel_start();
uint32_t get_kernel_end();

uint32_t get_kernel_heap_start();
uint32_t get_kernel_heap_end();

uint32_t get_kernel_pmm_bitmap_start();

#endif // MISTIX_KERNEL_MEMORY_REGIONS_H