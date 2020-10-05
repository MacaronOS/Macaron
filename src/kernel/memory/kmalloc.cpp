#include "kmalloc.hpp"
#include "regions.hpp"

#include "../types.hpp"

void kmalloc_init()
{
    kmalloc_header_t* first_block = (kmalloc_header_t*)(get_kernel_heap_start());

    first_block->free = 1;
    first_block->next_block = 0;
    first_block->prev_block = 0;
    first_block->size = get_kernel_heap_end() - get_kernel_heap_start() - sizeof(kmalloc_header_t);
}

void* kmalloc(size_t size)
{
    kmalloc_header_t* block = (kmalloc_header_t*)(get_kernel_heap_start());
    kmalloc_header_t* first_fit_block = 0;

    while (!first_fit_block && block) {
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

    kmalloc_header_t* separeted_block = (kmalloc_header_t*)((uint32_t)first_fit_block + sizeof(kmalloc_header_t) + size);

    first_fit_block->size = size;
    first_fit_block->free = 0;
    first_fit_block->next_block = separeted_block;

    separeted_block->free = 1;
    separeted_block->size = copy_size - first_fit_block->size - sizeof(kmalloc_header_t);
    separeted_block->next_block = copy_next_block;
    separeted_block->prev_block = first_fit_block;

    return (void*)((uint32_t)first_fit_block + sizeof(kmalloc_header_t));
}

void kfree(void* mem)
{
    if ((uint32_t)mem + sizeof(kmalloc_header_t) >= get_kernel_heap_end() || (uint32_t)mem < get_kernel_heap_start()) {
        return;
    }
    kmalloc_header_t* block = (kmalloc_header_t*)((uint32_t)mem - sizeof(kmalloc_header_t));

    if (block->prev_block != 0 && block->prev_block->free == 1) {
        block->prev_block->size += block->size + sizeof(kmalloc_header_t);
        block->prev_block->next_block = block->next_block;
    }

    else if (block->next_block != 0 && block->next_block->free == 1) {
        block->free = 1;
        block->size += block->next_block->size + sizeof(kmalloc_header_t);
        block->next_block = block->next_block->next_block;
    }

    else {
        block->free = 1;
    }
}

#ifdef DEBUG
void kmalloc_dump()
{
    kmalloc_header_t* block = (kmalloc_header_t*)(get_kernel_heap_start());
    while (block) {
        term_print("block_size: ");
        term_printd(block->size);
        term_print(", block_free: ");
        term_printd(block->free);
        term_print("\n");
        block = block->next_block;
    }
}
#endif