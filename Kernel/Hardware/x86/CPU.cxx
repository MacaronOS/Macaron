#include "../CPU.hpp"

namespace Kernel {

void CPU::flush_tlb(uintptr_t virtual_address, size_t page_count)
{
    while (page_count) {
        asm volatile(
            "invlpg (%0)" ::"r"(virtual_address)
            : "memory");

        virtual_address += page_size();
        page_count--;
    }
}

}