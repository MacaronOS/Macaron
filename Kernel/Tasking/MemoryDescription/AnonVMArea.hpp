#pragma once

#include <Tasking/MemoryDescription/VMArea.hpp>

namespace Kernel::Tasking {

/*
Simplest anonymous memory mapping which is not backed by any device or file and belongs to only one process.
When a process attempts to write to this area, it allocates a page for that address.

Processes created via fork inherit this area as a unique individual copy.
*/

class AnonVMArea : public VMArea {
public:
    AnonVMArea(MemoryDescription& md, size_t vm_start, size_t vm_end, uint32_t flags)
        : VMArea(md, vm_start, vm_end, flags)
    {
    }

    virtual PageFaultStatus fault(size_t address) override;
    virtual void fork(MemoryDescription& other) override;
};

}