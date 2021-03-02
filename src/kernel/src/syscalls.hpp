#pragma once
#include "hardware/InterruptManager.hpp"
#include "hardware/trapframe.hpp"

namespace kernel::syscalls {

enum class SyscallSelector {
    Putc = 0,
    Exit,
    Fork,
    Read,
    Write,
    Open,
    Close,

    Execve = 9,

    Printd,

    END,
};

constexpr uint8_t syscall_count = (uint8_t)SyscallSelector::END;

class SyscallsManager : public InterruptHandler {
public:
    SyscallsManager();
    static void initialize();

    void handle_interrupt(trapframe_t* tf) override;
    void register_syscall(SyscallSelector ss, uint32_t syscall_ptr);

private:
    uint32_t m_syscalls[syscall_count];
};

}