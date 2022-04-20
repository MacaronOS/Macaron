#include "MemoryDescription.hpp"
#include <Memory/vmm.hpp>

namespace Kernel::Tasking {
using namespace Memory;

MemoryDescription::MemoryDescription()
    : m_memory_descriptor(VMM::the().create_page_directory())
{
}

void MemoryDescription::fork_from(MemoryDescription& other)
{
    for (auto area : other.m_memory_areas) {
        area->fork(*this);
    }
}

VMArea* MemoryDescription::find_memory_area_for(size_t address)
{
    for (auto& area : m_memory_areas) {
        if (address >= area->vm_start() && address < area->vm_end()) {
            return area;
        }
    }

    return nullptr;
}

void MemoryDescription::free_memory()
{
    auto last_area = m_memory_areas.rbegin();
    while (last_area != m_memory_areas.rend()) {
        auto& area = *last_area;
        VMM::the().psized_unmap(m_memory_descriptor, area->vm_start() / PAGE_SIZE, (area->vm_end() - area->vm_start()) / PAGE_SIZE);
        delete *last_area;
        last_area = m_memory_areas.remove(last_area);
    }
}

MemoryDescription kernel_memory_description((uint32_t)&boot_page_directory);
void init_kernel_memory_description()
{
    // When booted we mapped 8MB of the kernel virtual memory starting from HIGHER_HALF_OFFSET.
    // See Libkernel/Init.cpp.
    // Save information about that mapping in kernel_memory_description object.
    size_t kernel_memory_start = HIGHER_HALF_OFFSET;
    size_t kernel_memory_size = 8 * 1024 * 1024;
    kernel_memory_description.allocate_memory_area_from<VMArea>(
        kernel_memory_start,
        kernel_memory_size,
        VM_READ | VM_WRITE | VM_EXEC);
}

}