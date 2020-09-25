#ifndef MISTIX_KERNEL_DESCRIPTOR_ISR_H
#define MISTIX_KERNEL_DESCRIPTOR_ISR_H

#include "types.hpp"

typedef struct
{
    uint32_t ds; // Data segment selector
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // Pushed by pusha.
    uint32_t int_no, err_code; // Interrupt number and error code (if applicable)
    uint32_t eip, cs, eflags, useresp, ss; // Pushed by the processor automatically.
} registers_t;

typedef void (*isr_handler_t)(registers_t);

void register_interrupt_handler(uint8_t n, isr_handler_t handler);

#endif // MISTIX_KERNEL_DESCRIPTOR_TABLES_H