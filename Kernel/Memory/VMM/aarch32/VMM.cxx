#include "../VMM.hpp"

namespace Kernel::Memory {

uintptr_t VMM::page_fault_linear_address() const
{
    return 0;
}

uintptr_t VMM::current_translation_table() const
{
    return 0;
}

uintptr_t VMM::create_translation_table()
{
    return 0;
}

void VMM::set_translation_table(uintptr_t translation_table_physical_address)
{
    return;
}

void VMM::allocate_pages_from(size_t page, size_t pages, uint32_t flags)
{
    return;
}

void VMM::copy_pages_cow(uintptr_t translation_table_from, size_t page, size_t pages)
{
    return;
}

void VMM::map_pages(size_t page, size_t frame, size_t pages, uint32_t flags)
{
    return;
}

void VMM::unmap_pages(size_t page, size_t pages)
{
    return;
}

void VMM::handle_interrupt(Trapframe* tf)
{
    return;
}

}