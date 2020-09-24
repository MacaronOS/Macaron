#ifndef MISTIX_KERNEL_MEMORY_H
#define MISTIX_KERNEL_MEMORY_H

#include "types.h"

void* memset(void* ptr, int value, size_t num);
void* memcpy(uint8_t* write, uint8_t* read, size_t num);

#endif // MISTIX_KERNEL_MEMORY_H