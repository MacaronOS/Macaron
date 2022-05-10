#include "MemoryDescription.hpp"
#include <Memory/VMM/VMM.hpp>

namespace Kernel::Tasking {

using namespace Memory;

class TranslationTableRestorer {
public:
    TranslationTableRestorer()
        : m_last_translation_table(VMM::the().current_translation_table())
    {
    }

    ~TranslationTableRestorer()
    {
        VMM::the().set_translation_table(m_last_translation_table);
    }

private:
    uintptr_t m_last_translation_table;
};

MemoryDescription::MemoryDescription()
    : m_memory_descriptor(VMM::the().create_translation_table())
{
}

void MemoryDescription::fork_from(MemoryDescription& other)
{
    TranslationTableRestorer _;
    VMM::the().set_translation_table(m_memory_descriptor);

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
    TranslationTableRestorer _;
    VMM::the().set_translation_table(m_memory_descriptor);

    auto last_area = m_memory_areas.rbegin();
    while (last_area != m_memory_areas.rend()) {
        auto& area = *last_area;
        VMM::the().unmap_memory(area->vm_start(), area->vm_size());
        delete *last_area;
        last_area = m_memory_areas.remove(last_area);
    }
}

MemoryDescription kernel_memory_description;
void init_kernel_memory_description()
{
    // When booted we mapped 12MB of the kernel virtual memory starting from HIGHER_HALF_OFFSET.
    // See Libkernel/Init.cpp.
    // Save information about that mapping in kernel_memory_description object.
    size_t kernel_memory_start = HIGHER_HALF_OFFSET;
    size_t kernel_memory_size = 12 * 1024 * 1024;
    kernel_memory_description.allocate_memory_area_from<VMArea>(
        kernel_memory_start,
        kernel_memory_size,
        VM_READ | VM_WRITE | VM_EXEC);
}

}