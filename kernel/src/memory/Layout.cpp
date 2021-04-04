#include "Layout.hpp"

namespace kernel::memory {

extern "C" uint32_t _kernel_start;
extern "C" uint32_t _kernel_text_start;
extern "C" uint32_t _kernel_text_end;

extern "C" uint32_t _kernel_rodata_start;
extern "C" uint32_t _kernel_rodata_end;

extern "C" uint32_t _kernel_data_start;
extern "C" uint32_t _kernel_data_end;

extern "C" uint32_t _kernel_bss_start;
extern "C" uint32_t _kernel_bss_end;

extern "C" uint32_t _kernel_stack_start;
extern "C" uint32_t _kernel_stack_end;

extern "C" uint32_t _kernel_end;

static uint32_t LayoutElement2Location[] = {
    [static_cast<uint32_t>(LayoutElement::KernelStart)] = reinterpret_cast<uint32_t>(&_kernel_start),
    [static_cast<uint32_t>(LayoutElement::KernelStackStart)] = reinterpret_cast<uint32_t>(&_kernel_stack_start),
    [static_cast<uint32_t>(LayoutElement::KernelStackEnd)] = reinterpret_cast<uint32_t>(&_kernel_stack_end),
    [static_cast<uint32_t>(LayoutElement::KernelEnd)] = reinterpret_cast<uint32_t>(&_kernel_end),
    [static_cast<uint32_t>(LayoutElement::KernelHeapStart)] = reinterpret_cast<uint32_t>(&_kernel_end),
    [static_cast<uint32_t>(LayoutElement::KernelHeapEnd)] = reinterpret_cast<uint32_t>(&_kernel_end) + HEAP_SIZE,
    [static_cast<uint32_t>(LayoutElement::PagingBuffer1)] = reinterpret_cast<uint32_t>(&_kernel_end) + HEAP_SIZE,
    [static_cast<uint32_t>(LayoutElement::PagingBuffer2)] = reinterpret_cast<uint32_t>(&_kernel_end) + HEAP_SIZE + PAGING_BUFFER_SIZE,
    [static_cast<uint32_t>(LayoutElement::PMMBitmapStart)] = reinterpret_cast<uint32_t>(&_kernel_end) + HEAP_SIZE + 2 * PAGING_BUFFER_SIZE,
    [static_cast<uint32_t>(LayoutElement::PMMBitmapEnd)] = 0,
};

uint32_t Layout::GetLocationVirt(LayoutElement element)
{
    return LayoutElement2Location[static_cast<uint32_t>(element)];
}

uint32_t Layout::GetLocationPhys(LayoutElement element)
{
    return GetLocationVirt(element) - HIGHER_HALF_OFFSET;
}

void Layout::SetLocationVirt(LayoutElement element, uint32_t location)
{
    LayoutElement2Location[static_cast<uint32_t>(element)] = location;
}

}