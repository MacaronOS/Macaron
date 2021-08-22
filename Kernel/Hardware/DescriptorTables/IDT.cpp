
#include "IDT.hpp"
#include "IDTPointers.hpp"
#include "GDT.hpp"

#include <Hardware/Port.hpp>

#include <Wisterialib/Common.hpp>
#include <Wisterialib/Memory.hpp>

namespace Kernel::DescriptorTables::IDT {

extern "C" void idt_flush(uint32_t);

class [[gnu::packed]] IDTEntry {
public:
    IDTEntry() = default;
    void setup(uint32_t jmp_pointer, uint16_t segment, uint8_t flags)
    {
        m_jmp_pointer_lower = jmp_pointer & 0xFFFF;
        m_jmp_pointer_high = (jmp_pointer >> 16) & 0xFFFF;
        m_segment = segment;
        m_flags = flags;
    }

private:
    uint16_t m_jmp_pointer_lower {};
    uint16_t m_segment {};
    uint8_t m_padding {};
    uint8_t m_flags {};
    uint16_t m_jmp_pointer_high {};
};

struct [[gnu::packed]] IDTPointer {
    uint16_t limit_byte;
    uint32_t pointer;
};

static IDTEntry idt_entries[256] {};
static IDTPointer idt_pointer;

void Setup()
{
    idt_pointer.limit_byte = sizeof(IDTEntry) * 256 - 1;
    idt_pointer.pointer = (uint32_t)&idt_entries;
    
    // Remap the irq table.
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0x0);
    outb(0xA1, 0x0);

    // Setup isr + irq pointers
    for (size_t at = 0 ; at <= 47 ; at++) {
        idt_entries[at].setup(GetInitialInterruptPointer(at), GDT::KernelCodeOffset, 0x8E);
    }

    // Setup syscall interrupt pointer
    idt_entries[128].setup((uint32_t)isr128, GDT::KernelCodeOffset, 0xEE);

    idt_flush((uint32_t)&idt_pointer);
}

}