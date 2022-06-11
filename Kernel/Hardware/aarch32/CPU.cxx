#include "../CPU.hpp"

namespace Kernel {

void CPU::flush_tlb(uintptr_t virtual_address, size_t page_count)
{
    while (page_count) {
        asm volatile(
            "mcr p15, 0, %0, c8, c7, 3" ::"r"(virtual_address)
            : "memory");

        virtual_address += page_size();
        page_count--;
    }
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