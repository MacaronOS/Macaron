#include "CPU.hpp"

namespace Kernel {

void CPU::flush_tlb(uintptr_t virtual_address, size_t page_count)
{
    while (page_count) {
        asm volatile(
            "invlpg %0"
            :
            : "m"(virtual_address)
            : "memory");

        page_count--;
    }
}

}