#include "kmalloc.hpp"
#include "Layout.hpp"

#include "../assert.hpp"
#include "../types.hpp"

#include <monitor.hpp>

using namespace kernel::memory;

struct kmalloc_header_t {
    kmalloc_header_t* next_block;
    kmalloc_header_t* prev_block;
    size_t size;
    bool free;
};

void kmalloc_init()
{
    auto* first_block = reinterpret_cast<kmalloc_header_t*>(Layout::GetLocationVirt(LayoutElement::KernelHeapStart));
    first_block->size = Layout::GetLocationVirt(LayoutElement::KernelHeapEnd) - Layout::GetLocationVirt(LayoutElement::KernelHeapStart) - sizeof(kmalloc_header_t);
    first_block->free = true;
    first_block->next_block = nullptr;
    first_block->prev_block = nullptr;
}

void* kmalloc(size_t size)
{
    kmalloc_header_t* block = reinterpret_cast<kmalloc_header_t*>(Layout::GetLocationVirt(LayoutElement::KernelHeapStart));
    kmalloc_header_t* first_fit_block = nullptr;

    while (!first_fit_block && block) {
        if (block->free && block->size >= size + sizeof(kmalloc_header_t)) {
            first_fit_block = block;
        }
        block = block->next_block;
    }

    if (!first_fit_block) {
        ASSERT_PANIC("KMALLOC: OUT OF MEMORY");
    }

    kmalloc_header_t* copy_next_block = first_fit_block->next_block;
    size_t copy_size = first_fit_block->size;

    kmalloc_header_t* separeted_block = (kmalloc_header_t*)((uint32_t)first_fit_block + sizeof(kmalloc_header_t) + size);

    first_fit_block->size = size;
    first_fit_block->free = false;
    first_fit_block->next_block = separeted_block;

    separeted_block->free = true;
    separeted_block->size = copy_size - size - sizeof(kmalloc_header_t);
    separeted_block->next_block = copy_next_block;
    separeted_block->prev_block = first_fit_block;

    if (separeted_block->next_block) {
        separeted_block->next_block->prev_block = separeted_block;
    }

    return (void*)((uint32_t)first_fit_block + sizeof(kmalloc_header_t));
}

void* kmalloc_4(size_t size)
{
    return (void*)(((uint32_t)kmalloc(size + 3) + 3) & ~(uint32_t)(3));
}

void kfree(void* mem)
{
    kmalloc_header_t* block = (kmalloc_header_t*)((uint32_t)mem - sizeof(kmalloc_header_t));

    if ((uint32_t)block >= Layout::GetLocationVirt(LayoutElement::KernelHeapEnd) || (uint32_t)block < Layout::GetLocationVirt(LayoutElement::KernelHeapStart)) {
        return;
    }

    block->free = true;

    if (block && block->prev_block && block->prev_block->free) {
        block->prev_block->size += block->size + sizeof(kmalloc_header_t);
        block->prev_block->next_block = block->next_block;
        if (block->next_block) {
            block->next_block->prev_block = block->prev_block;
        }
        block = block->prev_block;
    }

    if (block && block->next_block && block->next_block->free) {
        block->size += block->next_block->size + sizeof(kmalloc_header_t);
        block->next_block = block->next_block->next_block;
        if (block->next_block) {
            block->next_block->prev_block = block;
        }
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