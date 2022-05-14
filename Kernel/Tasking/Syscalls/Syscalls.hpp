#pragma once
#include <Hardware/Interrupts/InterruptManager.hpp>
#include <Hardware/Trapframe.hpp>

#include <Macaronlib/ABI/Syscalls.hpp>

namespace Kernel::Syscalls {

constexpr auto syscall_count = (uint16_t)Syscall::END;

class SyscallsManager : public InterruptHandler {
public:
    SyscallsManager();
    static void initialize();

    void handle_interrupt(Trapframe* tf) override;
    void register_syscall(Syscall ss, uintptr_t syscall_ptr);

private:
    uintptr_t m_syscalls[syscall_count] {};
};

}