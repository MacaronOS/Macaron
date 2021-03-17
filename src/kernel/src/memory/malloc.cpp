#include "malloc.hpp"
#include "Layout.hpp"
#include "../assert.hpp"
#include <wisterialib/common.hpp>
#include <monitor.hpp>

using namespace kernel::memory;

struct malloc_header_t {
    malloc_header_t* next_block;
    malloc_header_t* prev_block;
    size_t size;
    bool free;
};

void malloc_init()
{
    auto* first_block = reinterpret_cast<malloc_header_t*>(Layout::GetLocationVirt(LayoutElement::KernelHeapStart));
    first_block->size = Layout::GetLocationVirt(LayoutElement::KernelHeapEnd) - Layout::GetLocationVirt(LayoutElement::KernelHeapStart) - sizeof(malloc_header_t);
    first_block->free = true;
    first_block->next_block = nullptr;
    first_block->prev_block = nullptr;
}

void* malloc(size_t size)
{
    malloc_header_t* block = reinterpret_cast<malloc_header_t*>(Layout::GetLocationVirt(LayoutElement::KernelHeapStart));
    malloc_header_t* first_fit_block = nullptr;

    while (!first_fit_block && block) {
        if (block->free && block->size >= size + sizeof(malloc_header_t)) {
            first_fit_block = block;
        }
        block = block->next_block;
    }

    if (!first_fit_block) {
        ASSERT_PANIC("malloc: OUT OF MEMORY");
    }

    malloc_header_t* copy_next_block = first_fit_block->next_block;
    size_t copy_size = first_fit_block->size;

    malloc_header_t* separeted_block = (malloc_header_t*)((uint32_t)first_fit_block + sizeof(malloc_header_t) + size);

    first_fit_block->size = size;
    first_fit_block->free = false;
    first_fit_block->next_block = separeted_block;

    separeted_block->free = true;
    separeted_block->size = copy_size - size - sizeof(malloc_header_t);
    separeted_block->next_block = copy_next_block;
    separeted_block->prev_block = first_fit_block;

    if (separeted_block->next_block) {
        separeted_block->next_block->prev_block = separeted_block;
    }

    return (void*)((uint32_t)first_fit_block + sizeof(malloc_header_t));
}

void* malloc_4(size_t size)
{
    return (void*)(((uint32_t)malloc(size + 3) + 3) & ~(uint32_t)(3));
}

void free(void* mem)
{
    malloc_header_t* block = (malloc_header_t*)((uint32_t)mem - sizeof(malloc_header_t));

    if ((uint32_t)block >= Layout::GetLocationVirt(LayoutElement::KernelHeapEnd) || (uint32_t)block < Layout::GetLocationVirt(LayoutElement::KernelHeapStart)) {
        return;
    }

    block->free = true;

    if (block && block->prev_block && block->prev_block->free) {
        block->prev_block->size += block->size + sizeof(malloc_header_t);
        block->prev_block->next_block = block->next_block;
        if (block->next_block) {
            block->next_block->prev_block = block->prev_block;
        }
        block = block->prev_block;
    }

    if (block && block->next_block && block->next_block->free) {
        block->size += block->next_block->size + sizeof(malloc_header_t);
        block->next_block = block->next_block->next_block;
        if (block->next_block) {
            block->next_block->prev_block = block;
        }
    }
}

#ifdef DEBUG
void malloc_dump()
{
    malloc_header_t* block = (malloc_header_t*)(Layout::GetLocationVirt(LayoutElement::KernelHeapStart));
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