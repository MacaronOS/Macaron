#pragma once

#include "../types.hpp"

typedef struct kmalloc_header {
    struct kmalloc_header* next_block;
    struct kmalloc_header* prev_block;
    size_t size;
    bool free;
} kmalloc_header_t;

void kmalloc_init();
void* kmalloc(size_t);

//kmalloc, but 4 bytes aligned
void* kmalloc_4(size_t);

void kfree(void*);
#ifdef DEBUG
void kmalloc_dump();
#endif

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

// inline void* operator new(unsigned long, unsigned long);
