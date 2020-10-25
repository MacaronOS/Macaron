#pragma once

#include "types.hpp"

struct [[gnu::packed]] registers_t
{
    uint32_t ds; // Data segment selector
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // Pushed by pusha.
    uint32_t int_no, err_code; // Interrupt number and error code (if applicable)
    uint32_t eip, cs, eflags, useresp, ss; // Pushed by the processor automatically.
};

typedef void (*isr_handler_t)(registers_t*);

void register_interrupt_handler(uint8_t n, isr_handler_t handler);