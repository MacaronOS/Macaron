#include "kmalloc.hpp"
#include "Layout.hpp"

#include "../assert.hpp"
#include "../types.hpp"

using namespace kernel::memory;

void kmalloc_init()
{
    kmalloc_header_t* first_block = reinterpret_cast<kmalloc_header_t*>(Layout::GetLocationVirt(LayoutElement::KernelHeapStart));
    first_block->size = Layout::GetLocationVirt(LayoutElement::KernelHeapEnd) - Layout::GetLocationVirt(LayoutElement::KernelHeapStart) - sizeof(kmalloc_header_t);
    first_block->free = 1;
    first_block->next_block = 0;
    first_block->prev_block = 0;
}

void* kmalloc(size_t size)
{
    kmalloc_header_t* block = reinterpret_cast<kmalloc_header_t*>(Layout::GetLocationVirt(LayoutElement::KernelHeapStart));
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

void* kmalloc_4(size_t size)
{
    return (void*)(((uint32_t)kmalloc(size + 3) + 3) & ~(uint32_t)(3));
}

void kfree(void* mem)
{
    if ((uint32_t)mem + sizeof(kmalloc_header_t) >= Layout::GetLocationVirt(LayoutElement::KernelHeapEnd) || (uint32_t)mem < Layout::GetLocationVirt(LayoutElement::KernelHeapStart)) {
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
    kmalloc_header_t* block = (kmalloc_header_t*)(Layout::GetLocationVirt(LayoutElement::KernelHeapStart));
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