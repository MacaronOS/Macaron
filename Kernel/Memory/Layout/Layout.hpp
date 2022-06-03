#pragma once
#include <Macaronlib/Common.hpp>

namespace Kernel::Memory {

constexpr uintptr_t HIGHER_HALF_OFFSET = 0xC0000000;
#ifdef __i386__
constexpr uintptr_t PHYSICAL_OFFSET = 0;
#else
constexpr uintptr_t PHYSICAL_OFFSET = 0x80000000;
#endif

enum class LayoutElement {
    KernelStart,
    KernelStackStart,
    KernelStackEnd,
    KernelEnd,

    TranslationAllocatorAreaStart,
    TranslationAllocatorAreaEnd,
    PagingBuffer1,
    PagingBuffer2,

    KernelInitialHeapStart,
    KernelInitialHeapEnd,
};

namespace Layout {
    uintptr_t GetLocationPhys(LayoutElement element);
    uintptr_t GetLocationVirt(LayoutElement element);

    uintptr_t VirtToPhys(uintptr_t addr);
    uintptr_t PhysToVirt(uintptr_t addr);
};

}