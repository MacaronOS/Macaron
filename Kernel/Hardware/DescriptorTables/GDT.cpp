#include "GDT.hpp"

namespace Kernel::DescriptorTables::GDT {

extern "C" void gdt_flush(uint32_t);
extern "C"  void tss_flush();

extern "C" uint32_t stack_top;


class [[gnu::packed]] GDTEntry {
public:
    void setup(uint32_t pointer, uint32_t limit_byte, uint32_t access, uint8_t granularity) 
    {
        m_pointer_low = (pointer & 0xFFFF);
        m_pointer_middle = (pointer >> 16) & 0xFF;
        m_pointer_high = (pointer >> 24) & 0xFF;

        m_limit_byte_low = (limit_byte & 0xFFFF);
        m_granularity = (limit_byte >> 16) & 0x0F;

        m_granularity |= granularity & 0xF0;
        m_access = access;
    }
    uint16_t m_limit_byte_low;
    uint16_t m_pointer_low;
    uint8_t m_pointer_middle;
    uint8_t m_access;
    uint8_t m_granularity;
    uint8_t m_pointer_high;
};

struct [[gnu::packed]] GDTPointer {
    uint16_t limit_byte;
    uint32_t pointer;
};

struct [[gnu::packed]] TSSEntry
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

static GDTEntry gdt_entries[6];
static GDTPointer gdt_pointer;
static TSSEntry tss_entry {};

void Setup()
{
    gdt_pointer.limit_byte = sizeof(gdt_entries) - 1;
    gdt_pointer.pointer = (uint32_t)&gdt_entries;

    gdt_entries[Null].setup(0, 0, 0, 0);
    gdt_entries[KernelCode].setup(0, 0xFFFFFFFF, 0x9A, 0xCF);
    gdt_entries[KernelData].setup(0, 0xFFFFFFFF, 0x92, 0xCF);
    gdt_entries[UserCode].setup(0, 0xFFFFFFFF, 0xFA, 0xCF);
    gdt_entries[UserData].setup(0, 0xFFFFFFFF, 0xF2, 0xCF);

    tss_entry.ss0 = KernelDataOffset;
    tss_entry.esp0 = (uint32_t)&stack_top;
    tss_entry.cs = KernelCodeOffset | 3;
    tss_entry.ss = KernelDataOffset | 3;
    tss_entry.ds = KernelDataOffset | 3;
    tss_entry.es = KernelDataOffset | 3;
    tss_entry.fs = KernelDataOffset | 3;
    tss_entry.gs = KernelDataOffset | 3;

    gdt_entries[TSS].setup((uint32_t)&tss_entry, (uint32_t)&tss_entry + sizeof(TSSEntry), 0xE9, 0x00);

    gdt_flush((uint32_t)&gdt_pointer);
    tss_flush();
}

void SetKernelStack(uint32_t stack) 
{
    tss_entry.esp0 = stack;
}

}