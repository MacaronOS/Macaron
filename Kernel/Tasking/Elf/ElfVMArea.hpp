#pragma once

#include <Filesystem/Base/Inode.hpp>
#include <Tasking/MemoryDescription/VMArea.hpp>

namespace Kernel::Tasking {

using namespace FileSystem;

class ElfVMArea : public VMArea {
public:
    ElfVMArea(MemoryDescription& md, size_t vm_start, size_t vm_end, uint32_t flags)
        : VMArea(md, vm_start, vm_end, flags)
    {
    }

    inline void setup(Inode* inode, size_t offset, size_t vaddr, size_t filesz, size_t memsz)
    {
        m_inode = inode;
        m_offset = offset;
        m_vaddr = vaddr;
        m_filesz = filesz;
        m_memsz = memsz;
    }

    virtual PageFaultStatus fault(size_t address) override;
    virtual void fork(MemoryDescription& other) override;

private:
    Inode* m_inode {};
    size_t m_offset {};
    size_t m_vaddr {};
    size_t m_filesz {};
    size_t m_memsz {};
};

}