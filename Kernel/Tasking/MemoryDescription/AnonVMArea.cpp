#include "AnonVMArea.hpp"
#include <Macaronlib/Memory.hpp>
#include <Memory/vmm.hpp>
#include <Tasking/MemoryDescription/MemoryDescription.hpp>

namespace Kernel::Tasking {

using namespace Memory;

AnonVMArea::PageFaultStatus AnonVMArea::fault(size_t address)
{
    size_t page_for_address = address / PAGE_SIZE;
    size_t page_aligned_address = page_for_address * PAGE_SIZE;

    VMM::the().psized_allocate_space_from(
        m_memory_description.memory_descriptor(),
        page_for_address,
        1,
        Flags::User | Flags::Write | Flags::Present);

    memset((void*)(page_aligned_address), 0, PAGE_SIZE);
    return PageFaultStatus::Handled;
}

// TODO: Implement fork as copy-on-write.
void AnonVMArea::fork(MemoryDescription& other)
{
    auto anon_vm_area = other.allocate_memory_area_from<AnonVMArea>(vm_start(), vm_end() - vm_start(), flags());
    if (!anon_vm_area) {
        return;
    }

    VMM::the().copy_allocated(other.memory_descriptor(),
        m_memory_description.memory_descriptor(),
        vm_start(), vm_end() - vm_start());
}

}