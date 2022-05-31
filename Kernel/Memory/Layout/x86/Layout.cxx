#include "../Layout.hpp"

namespace Kernel::Memory {

uintptr_t Layout::GetLocationPhys(LayoutElement element)
{
    return GetLocationVirt(element) - HIGHER_HALF_OFFSET;
}

uintptr_t Layout::VirtToPhys(uintptr_t addr)
{
    return addr - HIGHER_HALF_OFFSET;
}

uintptr_t Layout::PhysToVirt(uintptr_t addr)
{
    return addr + HIGHER_HALF_OFFSET;
}

}