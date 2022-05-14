#include "ElfVMArea.hpp"
#include <Memory/VMM/VMM.hpp>
#include <Tasking/MemoryDescription/MemoryDescription.hpp>

namespace Kernel::Tasking {

using namespace Memory;

ElfVMArea::PageFaultStatus ElfVMArea::fault(size_t address)
{
    if (address < m_vaddr || address >= m_vaddr + m_memsz) {
        return PageFaultStatus::Failed;
    }

    size_t page_for_address = address / CPU::page_size();
    size_t page_aligned_address = page_for_address * CPU::page_size();

    VMM::the().allocate_pages_from(
        page_for_address,
        1,
        Flags::User | Flags::Write | Flags::Present);

    size_t start = max(m_vaddr, page_aligned_address);
    size_t end = min(m_vaddr + m_memsz, start + CPU::page_size());
    size_t size = end - start;

    size_t file_start = start - m_vaddr + m_offset;
    if (file_start < m_offset + m_filesz) {
        size_t file_end = min(m_offset + m_filesz, end - m_vaddr + m_offset);
        size_t read_from_file = min(size, file_end - file_start);

        m_inode->read_bytes((void*)(start), read_from_file, file_start);
        start += read_from_file;
        size -= read_from_file;
    }

    memset((void*)(start), 0, size);

    return PageFaultStatus::Handled;
}

void ElfVMArea::fork(MemoryDescription& other)
{
    auto elf_vm_area = other.allocate_memory_area_from<ElfVMArea>(vm_start(), vm_end() - vm_start(), flags());
    if (!elf_vm_area) {
        return;
    }

    auto& area = *elf_vm_area.result();
    area.setup(m_inode, m_offset, m_vaddr, m_filesz, m_memsz);

    VMM::the().copy_memory_cow(
        m_memory_description.memory_descriptor(),
        vm_start(),
        vm_end() - vm_start());
}

}