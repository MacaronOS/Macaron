#ifndef MISTIX_KERNEL_KMALLOC_H
#define MISTIX_KERNEL_KMALLOC_H

#include "../types.hpp"
#include "regions.hpp"

#define HEAP_SIZE 4 * 1024 * 1024

typedef struct kmalloc_header {
    struct kmalloc_header* next_block;
    struct kmalloc_header* prev_block;
    size_t size;
    bool free;
} kmalloc_header_t;

void kmalloc_init();
uint32_t kmalloc(size_t);
void kfree(void*);

#endif // MISTIX_KERNEL_KMALLOC_H