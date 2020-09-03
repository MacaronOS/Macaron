#ifndef MISTIX_KERNEL_MEMORY_H
#define MISTIX_KERNEL_MEMORY_H

#include "types.h"

void* memset(void* ptr, int value, size_t num)
{
    for (size_t i = (size_t)ptr; i < (size_t)ptr + num; i++) {
        *(uint8_t*)i = 0;
    }
}

#endif // MISTIX_KERNEL_MEMORY_H