#ifndef MISTIX_KERNEL_DESCRIPTOR_TABLES_H
#define MISTIX_KERNEL_DESCRIPTOR_TABLES _H

#include "types.h"

typedef struct {
    uint16_t limit_low; // The lower 16 bits of the limit.
    uint16_t base_low; // The lower 16 bits of the base.
    uint8_t base_middle; // The next 8 bits of the base.
    uint8_t access; // Access flags, determine what ring this segment can be used in.
    uint8_t granularity;
    uint8_t base_high; // The last 8 bits of the base.
} __attribute__((packed)) gdt_entry_t;

typedef struct {
    uint16_t limit; // The upper 16 bits of all selector limits.
    uint32_t base; // The address of the first gdt_entry_t struct.
} __attribute__((packed)) gdt_ptr_t;

void init_descriptor_tables();

typedef struct {
    uint16_t base_lo; // the lower 16 bits of the address to jump to when this interrupt fires
    uint16_t sel; // kernel segment selector
    uint8_t null; // always zero
    uint8_t flags;
    uint16_t base_hi; // the upper 16 bits of the address to jump to
} __attribute__((packed)) idt_entry_t;

typedef struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) idt_ptr_t;

extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();

extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();


#endif // MISTIX_KERNEL_DESCRIPTOR_TABLES_H