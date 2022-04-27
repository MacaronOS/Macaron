#include "Malloc.hpp"
#include "Layout.hpp"
#include "vmm.hpp"

#include <Libkernel/Assert.hpp>
#include <Libkernel/Graphics/VgaTUI.hpp>
#include <Libkernel/Logger.hpp>
#include <Tasking/MemoryDescription/AnonVMArea.hpp>
#include <Tasking/MemoryDescription/MemoryDescription.hpp>

#include <Macaronlib/Vector.hpp>

using namespace Kernel::Memory;
using namespace Kernel::Tasking;

struct MallocHeader {
    MallocHeader* next;
    MallocHeader* prev;
    size_t size;
    bool free;
};

namespace Kernel::Memory {

constexpr size_t minimal_block_size = 4 * 4096;
Vector<MallocHeader*> additional_memory_blocks;

void SetupMalloc()
{
    auto first_block = reinterpret_cast<MallocHeader*>(Layout::GetLocationVirt(LayoutElement::KernelInitialHeapStart));
    first_block->size = Layout::GetLocationVirt(LayoutElement::KernelInitialHeapEnd) - Layout::GetLocationVirt(LayoutElement::KernelInitialHeapStart) - sizeof(MallocHeader);
    first_block->free = true;
    first_block->next = nullptr;
    first_block->prev = nullptr;
}

static MallocHeader* allocate_new_block(size_t size)
{
    size_t allocated_size = max(minimal_block_size, size + sizeof(MallocHeader));
    auto allocation_result = kernel_memory_description.allocate_memory_area<AnonVMArea>(allocated_size, VM_READ | VM_WRITE, true);
    if (!allocation_result) {
        return nullptr;
    }

    auto new_block = (MallocHeader*)allocation_result.result()->vm_start();
    new_block->size = allocated_size;
    new_block->next = nullptr;
    new_block->prev = nullptr;
    new_block->free = true;

    additional_memory_blocks.push_back(new_block);
    return new_block;
}

}

void* malloc(size_t size)
{
    auto find_first_fit_chunk = [size](MallocHeader* block) -> MallocHeader* {
        MallocHeader* first_fit_chunk = nullptr;
        MallocHeader* cur_chunk = block;

        while (!first_fit_chunk && cur_chunk) {
            if (cur_chunk->free && cur_chunk->size >= size + sizeof(MallocHeader)) {
                first_fit_chunk = cur_chunk;
            }
            cur_chunk = cur_chunk->next;
        }

        return first_fit_chunk;
    };

    auto initial_block = reinterpret_cast<MallocHeader*>(Layout::GetLocationVirt(LayoutElement::KernelInitialHeapStart));
    auto first_fit_chunk = find_first_fit_chunk(initial_block);

    for (size_t block = 0; !first_fit_chunk && block < additional_memory_blocks.size(); block++) {
        first_fit_chunk = find_first_fit_chunk(additional_memory_blocks[block]);
    }

    if (!first_fit_chunk) {
        first_fit_chunk = allocate_new_block(size);
        if (!first_fit_chunk) {
            ASSERT_PANIC("Malloc: OUT OF MEMORY");
        }
    }

    auto remain_chunk = (MallocHeader*)((uintptr_t)first_fit_chunk + size + sizeof(MallocHeader));
    remain_chunk->free = true;
    remain_chunk->size = first_fit_chunk->size - size - sizeof(MallocHeader);
    remain_chunk->next = first_fit_chunk->next;
    remain_chunk->prev = first_fit_chunk;

    if (remain_chunk->next) {
        remain_chunk->next->prev = remain_chunk;
    }

    first_fit_chunk->free = false;
    first_fit_chunk->size = size;
    first_fit_chunk->next = remain_chunk;

    return (void*)((uintptr_t)first_fit_chunk + sizeof(MallocHeader));
}

void* malloc_4(size_t size)
{
    return (void*)(((uintptr_t)malloc(size + 3) + 3) & ~(uintptr_t)(3));
}

void free(void* mem)
{
    if (!mem) {
        return;
    }

    MallocHeader* block = (MallocHeader*)((uintptr_t)mem - sizeof(MallocHeader));
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
