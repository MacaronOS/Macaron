#pragma once

#include <wisterialib/common.hpp>

#define GDT_KERNEL_CODE_OFFSET 0x08
#define GDT_KERNEL_DATA_OFFSET 0x10
#define GDT_USER_CODE_OFFSET 0x18
#define GDT_USER_DATA_OFFSET 0x20
#define GDT_TSS_OFFSET 0x28

#define GDT_KERNEL_CODE 1
#define GDT_KERNEL_DATA 2
#define GDT_USER_CODE 3
#define GDT_USER_DATA 4
#define GDT_TSS 5

#define REQUEST_RING_3 3

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
void write_tss(uint16_t ss0, uint32_t esp0);

void set_kernel_stack(uint32_t stack);

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

extern "C" void isr0();
extern "C" void isr1();
extern "C" void isr2();
extern "C" void isr3();
extern "C" void isr4();
extern "C" void isr5();
extern "C" void isr6();
extern "C" void isr7();
extern "C" void isr8();
extern "C" void isr9();
extern "C" void isr10();
extern "C" void isr11();
extern "C" void isr12();
extern "C" void isr13();
extern "C" void isr14();
extern "C" void isr15();
extern "C" void isr16();
extern "C" void isr17();
extern "C" void isr18();
extern "C" void isr19();
extern "C" void isr20();
extern "C" void isr21();
extern "C" void isr22();
extern "C" void isr23();
extern "C" void isr24();
extern "C" void isr25();
extern "C" void isr26();
extern "C" void isr27();
extern "C" void isr28();
extern "C" void isr29();
extern "C" void isr30();
extern "C" void isr31();

extern "C" void irq0();
extern "C" void irq1();
extern "C" void irq2();
extern "C" void irq3();
extern "C" void irq4();
extern "C" void irq5();
extern "C" void irq6();
extern "C" void irq7();
extern "C" void irq8();
extern "C" void irq9();
extern "C" void irq10();
extern "C" void irq11();
extern "C" void irq12();
extern "C" void irq13();
extern "C" void irq14();
extern "C" void irq15();
extern "C" void isr128();

struct [[gnu::packed]] tss_entry_t
{
    uint32_t prev_tss { 0 };
    uint32_t esp0 { 0 };
    uint32_t ss0 { 0 };
    uint32_t esp1 { 0 };
    uint32_t ss1 { 0 };
    uint32_t esp2 { 0 };
    uint32_t ss2 { 0 };
    uint32_t cr3 { 0 };
    uint32_t eip { 0 };
    uint32_t eflags { 0 };
    uint32_t eax { 0 };
    uint32_t ecx { 0 };
    uint32_t edx { 0 };
    uint32_t ebx { 0 };
    uint32_t esp { 0 };
    uint32_t ebp { 0 };
    uint32_t esi { 0 };
    uint32_t edi { 0 };
    uint32_t es { 0 };
    uint32_t cs { 0 };
    uint32_t ss { 0 };
    uint32_t ds { 0 };
    uint32_t fs { 0 };
    uint32_t gs { 0 };
    uint32_t ldt { 0 };
    uint16_t trap { 0 };
    uint16_t iomap_base { 0 };
};