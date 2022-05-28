#include "../TranslationAllocator.hpp"

namespace Kernel::Memory {

TranslationAllocator::TranslationAllocator()
{
    return;
}

void* TranslationAllocator::allocate_bytes(size_t bytes)
{
    return nullptr;
}

void TranslationAllocator::deallocate_bytes(void* address, size_t bytes)
{
    return;
}

uintptr_t TranslationAllocator::physical_to_virtual(uintptr_t physical_address)
{
    return 0;
}

uintptr_t TranslationAllocator::virtual_to_physical(uintptr_t virtual_address)
{
    return 0;
}

}