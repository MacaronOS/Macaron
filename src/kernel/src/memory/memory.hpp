#ifndef HinliXX_KERNEL_MEMORY_H
#define HinliXX_KERNEL_MEMORY_H

#include "../types.hpp"

void* memset(void* ptr, int value, size_t num);
void* memcpy(void* write, void* read, size_t num);

#endif // HinliXX_KERNEL_MEMORY_H