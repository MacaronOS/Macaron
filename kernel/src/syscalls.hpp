#pragma once
#include "hardware/InterruptManager.hpp"
#include "hardware/trapframe.hpp"

#include <wisterialib/posix/shared.hpp>

namespace kernel::syscalls {

constexpr uint8_t syscall_count = (uint8_t)Syscall::END;

class SyscallsManager : public InterruptHandler {
public:
    SyscallsManager();
    static void initialize();

    void handle_interrupt(trapframe_t* tf) override;
    void register_syscall(Syscall ss, uint32_t syscall_ptr);

private:
    uint32_t m_syscalls[syscall_count];
};

}