#include "../Layout.hpp"

namespace Kernel::Memory {

constexpr uintptr_t DDR2_OFFSET = 0x80000000;

uintptr_t Layout::GetLocationPhys(LayoutElement element)
{
    return GetLocationVirt(element) - HIGHER_HALF_OFFSET + DDR2_OFFSET;
}

uintptr_t Layout::VirtToPhys(uintptr_t addr)
{
    return addr - HIGHER_HALF_OFFSET + DDR2_OFFSET;
}

uintptr_t Layout::PhysToVirt(uintptr_t addr)
{
    return addr + HIGHER_HALF_OFFSET - DDR2_OFFSET;
}

}