#include "Malloc.hpp"
#include "Layout.hpp"

#include <Libkernel/Assert.hpp>
#include <Libkernel/Graphics/VgaTUI.hpp>

#include <Macaronlib/Common.hpp>

using namespace Kernel::Memory;

struct MallocHeader {
    MallocHeader* next;
    MallocHeader* prev;
    size_t size;
    bool free;
};

namespace Kernel::Memory {

void SetupMalloc()
{
    auto* first_block = reinterpret_cast<MallocHeader*>(Layout::GetLocationVirt(LayoutElement::KernelHeapStart));
    first_block->size = Layout::GetLocationVirt(LayoutElement::KernelHeapEnd) - Layout::GetLocationVirt(LayoutElement::KernelHeapStart) - sizeof(MallocHeader);
    first_block->free = true;
    first_block->next = nullptr;
    first_block->prev = nullptr;
}

}

void* malloc(size_t size)
{
    MallocHeader* block = reinterpret_cast<MallocHeader*>(Layout::GetLocationVirt(LayoutElement::KernelHeapStart));
    MallocHeader* first_fit_block = nullptr;

    while (!first_fit_block && block) {
        if (block->free && block->size >= size + sizeof(MallocHeader)) {
            first_fit_block = block;
        }
        block = block->next;
    }

    if (!first_fit_block) {
        ASSERT_PANIC("Malloc: OUT OF MEMORY");
    }

    MallocHeader* remain_block = (MallocHeader*)((uint32_t)first_fit_block + size + sizeof(MallocHeader));
    remain_block->free = true;
    remain_block->size = first_fit_block->size - size - sizeof(MallocHeader);
    remain_block->next = first_fit_block->next;
    remain_block->prev = first_fit_block;

    if (remain_block->next) {
        remain_block->next->prev = remain_block;
    }

    first_fit_block->free = false;
    first_fit_block->size = size;
    first_fit_block->next = remain_block;

    return (void*)((uint32_t)first_fit_block + sizeof(MallocHeader));
}

void* malloc_4(size_t size)
{
    return (void*)(((uint32_t)malloc(size + 3) + 3) & ~(uint32_t)(3));
}

void free(void* mem)
{
    MallocHeader* block = (MallocHeader*)((uint32_t)mem - sizeof(MallocHeader));

    if ((uint32_t)block >= Layout::GetLocationVirt(LayoutElement::KernelHeapEnd) || (uint32_t)block < Layout::GetLocationVirt(LayoutElement::KernelHeapStart)) {
        return;
    }

    block->free = true;

    if (block && block->prev && block->prev->free) {
        block->prev->size += block->size + sizeof(MallocHeader);
        block->prev->next = block->next;
        if (block->next) {
            block->next->prev = block->prev;
        }
        block = block->prev;
    }

    if (block && block->next && block->next->free) {
        block->size += block->next->size + sizeof(MallocHeader);
        block->next = block->next->next;
        if (block->next) {
            block->next->prev = block;
        }
    }
}
