#pragma once

#include <Tasking/MemoryDescription/VMArea.hpp>

namespace Kernel::Tasking {

class SharedVMArea : public VMArea {
public:
    SharedVMArea(MemoryDescription& md, size_t vm_start, size_t vm_end, uint32_t flags);

    virtual PageFaultStatus fault(size_t address) override;
    virtual void fork(MemoryDescription& other) override;

private:
    size_t m_pm_start {};
};

}