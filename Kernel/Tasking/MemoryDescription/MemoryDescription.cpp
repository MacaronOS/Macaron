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
        VMM::the().psized_free(m_memory_descriptor, area->vm_start() / PAGE_SIZE, (area->vm_end() - area->vm_start()) / PAGE_SIZE);
        delete *last_area;
        last_area = m_memory_areas.remove(last_area);
    }
}

}