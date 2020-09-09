#include "memory.h"
#include "types.h"

// end is defined in the linker script.
extern uint32_t end;
uint32_t placement_address = (uint32_t)&end;

void* memset(void* ptr, int value, size_t num)
{
    for (size_t i = (size_t)ptr; i < (size_t)ptr + num; i++) {
        *(uint8_t*)i = 0;
    }
}

uint32_t kmalloc(uint32_t sz, bool align, uint32_t* phys) {

    if (align && placement_address & 0xFFFFF000) {
        // the least significant 12 bits of the 32-bit word should always be zero
        placement_address &= 0xFFFFF000;
        placement_address += 0x1000;
    }
    if (phys) {
        *phys = placement_address;
    }
    uint32_t tmp = placement_address;
    placement_address += sz;
    return placement_address;
}

