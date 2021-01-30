#ifndef MISTIX_KERNEL_MEMORY_H
#define MISTIX_KERNEL_MEMORY_H

#include "../types.hpp"

void* memset(void* ptr, int value, size_t num);
void* memcpy(void* write, void* read, size_t num);

#endif // MISTIX_KERNEL_MEMORY_H