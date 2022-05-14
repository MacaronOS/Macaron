#pragma once

#include "Signals.hpp"
#include <Hardware/Trapframe.hpp>

namespace Kernel::Tasking {

constexpr size_t kernel_stack_size = 4096;
constexpr size_t user_stack_size = 512 * 4096;

// Not realy used inside the C++ code.
// Just to clarify the sequence of push / pop opertions
// inside Scheduling.s block routines.
struct KernelContext {
    uint32_t ebx;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
};

class Process;
class Thread {
    friend class Process;
    friend class Process;

public:
    Thread(Process& process, uintptr_t user_stack);
    ~Thread();

    Thread(const Thread&) = delete;
    Thread& operator=(const Thread&) = delete;

    inline Signals& signals() { return m_signals; }
    inline Process& process() { return m_process; }
    inline uintptr_t kernel_stack_top() { return m_kernel_stack + kernel_stack_size; }
    inline Trapframe* trapframe() { return (Trapframe*)(kernel_stack_top() - sizeof(Trapframe)); }
    inline KernelContext** kernel_context() { return &m_kernel_context; }
    inline bool blocked_in_kernel() { return m_kernel_context != nullptr; }

    void fork_from(Thread& other);
    void jump_to_signal_caller(int signo);
    void return_from_signal_caller();

private:
    void setup_trapframe();
    uintptr_t user_stack() { return m_user_stack; }
    uintptr_t user_stack_top() { return m_user_stack + user_stack_size; }

private:
    Process& m_process;
    Signals m_signals {};
    uintptr_t m_kernel_stack {};
    uintptr_t m_user_stack {};
    KernelContext* m_kernel_context {};
};

}