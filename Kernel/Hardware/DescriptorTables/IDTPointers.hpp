#pragma once

#include <Wisterialib/common.hpp>

namespace Kernel::DescriptorTables::IDT {

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

inline uint32_t GetInitialInterruptPointer(size_t interrupt_number)
{
    static uint32_t pointers[] = {
        (uint32_t)isr0,
        (uint32_t)isr1,
        (uint32_t)isr2,
        (uint32_t)isr3,
        (uint32_t)isr4,
        (uint32_t)isr5,
        (uint32_t)isr6,
        (uint32_t)isr7,
        (uint32_t)isr8,
        (uint32_t)isr9,
        (uint32_t)isr10,
        (uint32_t)isr11,
        (uint32_t)isr12,
        (uint32_t)isr13,
        (uint32_t)isr14,
        (uint32_t)isr15,
        (uint32_t)isr16,
        (uint32_t)isr17,
        (uint32_t)isr18,
        (uint32_t)isr19,
        (uint32_t)isr20,
        (uint32_t)isr21,
        (uint32_t)isr22,
        (uint32_t)isr23,
        (uint32_t)isr24,
        (uint32_t)isr25,
        (uint32_t)isr26,
        (uint32_t)isr27,
        (uint32_t)isr28,
        (uint32_t)isr29,
        (uint32_t)isr30,
        (uint32_t)isr31,
        (uint32_t)irq0,
        (uint32_t)irq1,
        (uint32_t)irq2,
        (uint32_t)irq3,
        (uint32_t)irq4,
        (uint32_t)irq5,
        (uint32_t)irq6,
        (uint32_t)irq7,
        (uint32_t)irq8,
        (uint32_t)irq9,
        (uint32_t)irq10,
        (uint32_t)irq11,
        (uint32_t)irq12,
        (uint32_t)irq13,
        (uint32_t)irq14,
        (uint32_t)irq15,
    };
    return pointers[interrupt_number];
}

}



