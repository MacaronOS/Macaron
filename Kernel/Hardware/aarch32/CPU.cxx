#include "../CPU.hpp"

namespace Kernel {

void CPU::flush_tlb(uintptr_t virtual_address, size_t page_count)
{
    // After the translation tables update, any old copies of entries that
    // might be held in the TLBs must be invalidated.
    while (page_count) {
        asm volatile(
            "mcr p15, 0, %0, c8, c7, 3" ::"r"(virtual_address)
            : "memory");

        virtual_address += page_size();
        page_count--;
    }
    // This operation is only guaranteed to affect all instructions,
    // including instruction fetches and data accesses,
    // after the execution of a DSB and an ISB.
    asm volatile("dsb");
    asm volatile("isb");
}

void CPU::enable_interrupts()
{
    asm volatile("cpsie i");
}

void CPU::disable_interrupts()
{
    asm volatile("cpsid i");
}

}