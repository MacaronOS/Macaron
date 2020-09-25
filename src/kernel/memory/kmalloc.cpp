#include "kmalloc.hpp"
#include "regions.hpp"

#include "../types.hpp"

void kmalloc_init()
{
    kmalloc_header_t* first_block = reinterpret_cast<kmalloc_header_t*>(get_kernel_heap_start());

    first_block->free = 1;
    first_block->next_block = 0;
    first_block->prev_block = 0;
    first_block->size = get_kernel_heap_end() - get_kernel_heap_start() - sizeof(kmalloc_header_t);
}

uint32_t kmalloc(size_t size)
{
    kmalloc_header_t* block = reinterpret_cast<kmalloc_header_t*>(get_kernel_heap_start());
    kmalloc_header_t* first_fit_block = 0;

    while (block && !first_fit_block) {
        if (block->free && block->size >= size + sizeof(kmalloc_header_t)) {
            first_fit_block = block;
        }
        block = block->next_block;
    }

    if (!first_fit_block) {
        return 0;
    }

    kmalloc_header_t* copy_next_block = first_fit_block->next_block;
    size_t copy_size = first_fit_block->size;

    kmalloc_header_t* separeted_block = first_fit_block + sizeof(kmalloc_header_t);

    first_fit_block->size = size;
    first_fit_block->free = 0;
    first_fit_block->next_block = separeted_block;

    separeted_block->free = 1;
    separeted_block->size = copy_size - first_fit_block->size - sizeof(kmalloc_header_t);
    separeted_block->next_block = copy_next_block;

    return (uint32_t)first_fit_block + sizeof(kmalloc_header_t);
}

void kfree(void* mem)
{
    kmalloc_header_t* block = reinterpret_cast<kmalloc_header_t*>((uint32_t)mem - sizeof(kmalloc_header_t));

    if (block->prev_block && block->prev_block->free) {
        block->prev_block->size += block->size + sizeof(kmalloc_header_t);
        block->prev_block->next_block = block->next_block;
    }

    else if (block->next_block && block->next_block->free) {
        block->free = 1;
        block->size += block->next_block->size + sizeof(kmalloc_header_t);
        block->next_block = block->next_block->next_block;
    }
}