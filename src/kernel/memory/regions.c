#include "regions.h"
#include "../types.h"

extern uint32_t _kernel_start;

extern uint32_t _kernel_text_start;
extern uint32_t _kernel_text_end;

extern uint32_t _kernel_rodata_start;
extern uint32_t _kernel_rodata_end;

extern uint32_t _kernel_data_start;
extern uint32_t _kernel_data_end;

extern uint32_t _kernel_bss_start;
extern uint32_t _kernel_bss_end;

extern uint32_t _kernel_end;

void* get_kernel_stack_start() {
    return 0;
}

void* get_kernel_stack_end() {
    return get_kernel_stack_start() + KERNEL_STACK_SIZE;
}

void* get_kernel_start() {
    return &_kernel_start;
}

void* get_kernel_end() {
    return &_kernel_end;
}

void* get_kernel_heap_start() {
    return get_kernel_end();
}

void* get_kernel_heap_end() {
    return get_kernel_heap_start() + KERNEL_HEAP_SIZE;
}

void* get_kernel_pmm_bitmap_start() {
    return get_kernel_heap_end();
}