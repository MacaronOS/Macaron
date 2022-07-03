#pragma once

#include <Macaronlib/Common.hpp>

namespace Kernel {

uintptr_t read_page_fault_linear_address()
{
    uintptr_t cr2;
    asm("mov %%cr2, %%eax"
        : "=a"(cr2));
    return cr2;
}

}