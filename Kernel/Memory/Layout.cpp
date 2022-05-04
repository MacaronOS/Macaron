#include "Layout.hpp"
#include <Macaronlib/Common.hpp>

namespace Kernel::Memory {

extern "C" uintptr_t _kernel_start;
extern "C" uintptr_t _kernel_text_start;
extern "C" uintptr_t _kernel_text_end;

extern "C" uintptr_t _kernel_rodata_start;
extern "C" uintptr_t _kernel_rodata_end;

extern "C" uintptr_t _kernel_data_start;
extern "C" uintptr_t _kernel_data_end;

extern "C" uintptr_t _kernel_bss_start;
extern "C" uintptr_t _kernel_stack_start;
extern "C" uintptr_t _kernel_stack_end;
extern "C" uintptr_t _kernel_bss_end;

extern "C" uintptr_t _kernel_end;

extern "C" uintptr_t _end;

static uintptr_t LayoutElement2Location[] = {
    [static_cast<uintptr_t>(LayoutElement::KernelStart)] = reinterpret_cast<uintptr_t>(&_kernel_start),
    [static_cast<uintptr_t>(LayoutElement::KernelStackStart)] = reinterpret_cast<uintptr_t>(&_kernel_stack_start),
    [static_cast<uintptr_t>(LayoutElement::KernelStackEnd)] = reinterpret_cast<uintptr_t>(&_kernel_stack_end),
    [static_cast<uintptr_t>(LayoutElement::KernelEnd)] = reinterpret_cast<uintptr_t>(&_kernel_end),

    [static_cast<uintptr_t>(LayoutElement::TranslationAllocatorAreaStart)] = reinterpret_cast<uintptr_t>(HIGHER_HALF_OFFSET + 4 * 1024 * 1024),
    [static_cast<uintptr_t>(LayoutElement::TranslationAllocatorAreaEnd)] = reinterpret_cast<uintptr_t>(HIGHER_HALF_OFFSET + 8 * 1024 * 1024 - 2 * 4096),
    [static_cast<uintptr_t>(LayoutElement::PagingBuffer1)] = reinterpret_cast<uintptr_t>(HIGHER_HALF_OFFSET + 8 * 1024 * 1024 - 2 * 4096),
    [static_cast<uintptr_t>(LayoutElement::PagingBuffer2)] = reinterpret_cast<uintptr_t>(HIGHER_HALF_OFFSET + 8 * 1024 * 1024 - 1 * 4096),

    [static_cast<uintptr_t>(LayoutElement::KernelInitialHeapStart)] = reinterpret_cast<uintptr_t>(HIGHER_HALF_OFFSET + 8 * 1024 * 1024),
    [static_cast<uintptr_t>(LayoutElement::KernelInitialHeapEnd)] = reinterpret_cast<uintptr_t>(HIGHER_HALF_OFFSET + 12 * 1024 * 1024),
};

uintptr_t Layout::GetLocationVirt(LayoutElement element)
{
    return LayoutElement2Location[static_cast<uintptr_t>(element)];
}

uintptr_t Layout::GetLocationPhys(LayoutElement element)
{
    return GetLocationVirt(element) - HIGHER_HALF_OFFSET;
}

uintptr_t Layout::VirtToPhys(uintptr_t addr)
{
    return addr - HIGHER_HALF_OFFSET;
}

uintptr_t Layout::PhysToVirt(uintptr_t addr)
{
    return addr + HIGHER_HALF_OFFSET;
}

}