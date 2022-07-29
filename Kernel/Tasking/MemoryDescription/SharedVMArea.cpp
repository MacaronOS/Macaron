#include "SharedVMArea.hpp"

#include <Memory/Utils.hpp>
#include <Memory/VMM/VMM.hpp>
#include <Memory/pmm.hpp>
#include <Tasking/MemoryDescription/MemoryDescription.hpp>

namespace Kernel::Tasking {

using namespace Memory;

SharedVMArea::SharedVMArea(MemoryDescription& md, size_t vm_start, size_t vm_end, uint32_t flags)
    : SharedVMAreaBase(md, vm_start, vm_end, flags)
{
    m_pm_start = page_to_address(PMM::the().allocate_frames(bytes_to_pages(vm_size())));
}

SharedVMAreaBase::PageFaultStatus SharedVMAreaBase::fault(size_t address)
{
    VMM::the().map_memory(
        address,
        m_pm_start,
        CPU::page_size(),
        Flags::User | Flags::Write | Flags::Present);

    return PageFaultStatus::Handled;
}

void SharedVMAreaBase::fork(MemoryDescription& other)
{
    auto shared_vm_area = other.allocate_memory_area_from<SharedVMAreaBase>(vm_start(), vm_size(), flags());
    if (!shared_vm_area) {
        return;
    }
    shared_vm_area.result()->m_pm_start = m_pm_start;
}

}