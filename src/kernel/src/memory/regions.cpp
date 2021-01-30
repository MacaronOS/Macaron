#include "regions.hpp"
#include "../types.hpp"

extern uint32_t _kernel_start;

extern uint32_t _kernel_text_start;
extern uint32_t _kernel_text_end;

extern uint32_t _kernel_rodata_start;
extern uint32_t _kernel_rodata_end;

extern uint32_t _kernel_data_start;
extern uint32_t _kernel_data_end;

extern uint32_t _kernel_bss_start;
extern uint32_t _kernel_bss_end;

extern uint32_t _kernel_stack_start;
extern uint32_t _kernel_stack_end;

extern uint32_t _kernel_end;

uint32_t get_kernel_start(bool phys)
{
    return reinterpret_cast<uint32_t>(&_kernel_start) - ((phys) ? HIGHER_HALF_OFFSET : 0);
}

uint32_t get_kernel_end(bool phys)
{
    return reinterpret_cast<uint32_t>(&_kernel_end) - ((phys) ? HIGHER_HALF_OFFSET : 0);
}

uint32_t get_kernel_stack_start(bool phys)
{
    return reinterpret_cast<uint32_t>(&_kernel_stack_start) - ((phys) ? HIGHER_HALF_OFFSET : 0);
}
uint32_t get_kernel_stack_end(bool phys)
{
    return reinterpret_cast<uint32_t>(&_kernel_stack_end) - ((phys) ? HIGHER_HALF_OFFSET : 0);
}

uint32_t get_kernel_heap_start(bool phys)
{
    return get_kernel_end(phys);
}

uint32_t get_kernel_heap_end(bool phys)
{
    return get_kernel_heap_start(phys) + KERNEL_HEAP_SIZE;
}

uint32_t get_kernel_pmm_bitmap_start(bool phys)
{
    return get_kernel_heap_end(phys);
}

uint32_t get_pd_temp_location()
{
    uint32_t bitmap_start = get_kernel_pmm_bitmap_start(false);
    return ((bitmap_start + 32 * 1024) / 4096 + 1) * 4096;
}

uint32_t get_pt_temp_location()
{
    return get_pd_temp_location() + 4096;
}