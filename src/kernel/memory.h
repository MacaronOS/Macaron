#ifndef MISTIX_KERNEL_MEMORY_H
#define MISTIX_KERNEL_MEMORY_H

#include "types.h"

void* memset(void* ptr, int value, size_t num);

uint32_t kmalloc(uint32_t sz, bool align, uint32_t* phys);

#endif // MISTIX_KERNEL_MEMORY_H