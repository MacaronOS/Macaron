#pragma once

#include <Tasking/MemoryDescription/VMArea.hpp>

namespace Kernel::Tasking {

class SharedVMAreaBase : public VMArea {
public:
    SharedVMAreaBase(MemoryDescription& md, size_t vm_start, size_t vm_end, uint32_t flags)
        : VMArea(md, vm_start, vm_end, flags)
    {
    }

    virtual PageFaultStatus fault(size_t address) override;
    virtual void fork(MemoryDescription& other) override;

protected:
    uintptr_t m_pm_start {};
};

class SharedVMArea : public SharedVMAreaBase {
public:
    SharedVMArea(MemoryDescription& md, size_t vm_start, size_t vm_end, uint32_t flags);

private:
    size_t m_pm_start {};
};

class ExplicitlySharedVMArea : public SharedVMAreaBase {
public:
    ExplicitlySharedVMArea(MemoryDescription& md, size_t vm_start, size_t vm_end, uint32_t flags)
        : SharedVMAreaBase(md, vm_start, vm_end, flags)
    {
    }

    void set_pm_start(uintptr_t pm_start) { m_pm_start = pm_start; }
};

}