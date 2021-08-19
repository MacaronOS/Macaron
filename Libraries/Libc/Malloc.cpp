#include "Malloc.hpp"

#include <Libc/Syscalls.hpp>
#include <Wisterialib/common.hpp>
#include <Wisterialib/posix/defines.hpp>

struct MallocHeader {
    MallocHeader* next;
    MallocHeader* prev;
    size_t size;
    bool free;
};

constexpr uint8_t MAX_MEM_BLOCKS = 64;
static MallocHeader* MEMORY[MAX_MEM_BLOCKS];
static uint8_t allocated_mem_blocks;

constexpr uint32_t minimal_block_size = 4 * 4096;

static MallocHeader* allocate_new_block(size_t size)
{
    if (allocated_mem_blocks >= MAX_MEM_BLOCKS) {
        return nullptr;
    }
    size_t allocated_size = max(minimal_block_size, size + sizeof(MallocHeader));
    // TODO: prot are ingored for now
    auto new_block = (MallocHeader*)mmap(nullptr, allocated_size, 0, MAP_ANONYMOUS, 0, 0);
    if (new_block == (void*)-1) {
        return nullptr;
    }
    new_block->size = allocated_size;
    new_block->next = nullptr;
    new_block->prev = nullptr;
    new_block->free = true;
    MEMORY[allocated_mem_blocks++] = new_block;
    return new_block;
}

void* malloc(size_t size)
{
    MallocHeader* first_fit_chunk = nullptr;

    for (size_t block = 0; !first_fit_chunk && block < allocated_mem_blocks; block++) {
        auto cur_chunk = (MallocHeader*)MEMORY[block];

        while (!first_fit_chunk && cur_chunk) {
            if (cur_chunk->free && cur_chunk->size >= size + sizeof(MallocHeader)) {
                first_fit_chunk = cur_chunk;
            }
            cur_chunk = cur_chunk->next;
        }
    }

    if (!first_fit_chunk) {
        first_fit_chunk = allocate_new_block(size);
        if (!first_fit_chunk) {
            exit(1);
        }
    }

    auto remain_chunk = (MallocHeader*)((uint32_t)first_fit_chunk + size + sizeof(MallocHeader));
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

    return (void*)((uint32_t)first_fit_chunk + sizeof(MallocHeader));
}

void free(void* mem)
{
    if ((uint32_t)mem == 0) {
        return;
    }
    auto chunk = (MallocHeader*)((uint32_t)mem - sizeof(MallocHeader));
    chunk->free = true;

    if (chunk->prev && chunk->prev->free) {
        chunk->prev->size += chunk->size + sizeof(MallocHeader);
        chunk->prev->next = chunk->next;
        if (chunk->next) {
            chunk->next->prev = chunk->prev;
        }
        chunk = chunk->prev;
    }

    if (chunk->next && chunk->next->free) {
        chunk->size += chunk->next->size + sizeof(MallocHeader);
        chunk->next = chunk->next->next;
        if (chunk->next) {
            chunk->next->prev = chunk;
        }
    }
}