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
void* kmalloc(size_t);
void kfree(void*);

inline void* operator new(unsigned long size)
{
    return kmalloc(size);
}

inline void operator delete(void* ptr)
{
    return kfree(ptr);
}

inline void operator delete(void* ptr, unsigned long)
{
    return kfree(ptr);
}

inline void* operator new[](unsigned long size)
{
    return kmalloc(size);
}

inline void operator delete[](void* ptr)
{
    return kfree(ptr);
}

inline void operator delete[](void* ptr, unsigned long)
{
    return kfree(ptr);
}

inline void* operator new(unsigned long, void* ptr)
{
    return ptr;
}

inline void* operator new[](unsigned long, void* ptr)
{
    return ptr;
}

#endif // MISTIX_KERNEL_KMALLOC_H