#include "AnonVMArea.hpp"
#include <Macaronlib/Memory.hpp>
#include <Memory/VMM/VMM.hpp>
#include <Tasking/MemoryDescription/MemoryDescription.hpp>

namespace Kernel::Tasking {

using namespace Memory;

AnonVMArea::PageFaultStatus AnonVMArea::fault(size_t address)
{
    size_t page_for_address = address / CPU::page_size();
    size_t page_aligned_address = page_for_address * CPU::page_size();

    VMM::the().allocate_pages_from(
        page_for_address,
        1,
        Flags::User | Flags::Write | Flags::Present);

    return PageFaultStatus::Handled;
}

void AnonVMArea::fork(MemoryDescription& other)
{
    auto anon_vm_area = other.allocate_memory_area_from<AnonVMArea>(vm_start(), vm_end() - vm_start(), flags());
    if (!anon_vm_area) {
        return;
    }

    VMM::the().copy_memory_cow(
        m_memory_description.memory_descriptor(),
        vm_start(),
        vm_end() - vm_start());
}

}