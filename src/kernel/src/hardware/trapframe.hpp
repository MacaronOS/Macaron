#pragma once

#include "../types.hpp"

struct [[gnu::packed]] trapframe_t
{
    uint32_t ds;
    uint32_t es;
    uint32_t fs;
    uint32_t gs;

    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // Pushed by pusha.

    uint32_t int_no, err_code; // Interrupt number and error code

    uint32_t eip, cs, eflags, useresp, ss; // Pushed by the processor automatically.
};