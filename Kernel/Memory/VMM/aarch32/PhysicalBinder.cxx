#include "../PhysicalBinder.hpp"

namespace Kernel::Memory {

PhysicalBinder::PhysicalBinder(uintptr_t virtual_base_address)
    : m_virtual_base_address(virtual_base_address)
{
    return;
}

void PhysicalBinder::bind_physical_page(uintptr_t physical_page_address)
{
    return;
}

void PhysicalBinder::unbind_physical_page()
{
    return;
}

}