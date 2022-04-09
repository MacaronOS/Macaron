#pragma once

#include <Macaronlib/Common.hpp>

namespace Kernel::Tasking {

constexpr auto VM_READ = 0x00000001;
constexpr auto VM_WRITE = 0x00000002;
constexpr auto VM_EXEC = 0x00000004;

class MemoryDescription;

class VMArea {
public:
    VMArea(MemoryDescription& md, size_t vm_start, size_t vm_end, uint32_t flags)
        : m_memory_description(md)
        , m_vm_start(vm_start)
        , m_vm_end(vm_end)
        , m_flags(flags)
    {
    }

    inline size_t vm_start() const { return m_vm_start; }
    inline size_t vm_end() const { return m_vm_end; }
    inline uint32_t flags() const { return m_flags; }

    enum class PageFaultStatus {
        Failed,
        Handled,
    };
    virtual PageFaultStatus fault(size_t address) { return PageFaultStatus::Failed; }
    virtual void fork(MemoryDescription& other) { }

protected:
    MemoryDescription& m_memory_description;

private:
    size_t m_vm_start;
    size_t m_vm_end;
    uint32_t m_flags;
};

}
