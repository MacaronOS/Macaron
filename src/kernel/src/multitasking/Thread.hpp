#pragma once
#include "../hardware/trapframe.hpp"

namespace kernel::multitasking {

#define KERNEL_STACK_SIZE 4096
#define USER_STACK_SIZE 4096

enum class ThreadState {
    New,
    Running,
    Ready,
    Terminated,
    Blocked,
};

class Process;

struct KernelContext {
    uint32_t edi; // eax, ecx,
    uint32_t esi;
    uint32_t ebx;
    uint32_t ebp;
    uint32_t esp;
    uint32_t eip;
};

struct Thread {
    Process* process;
    ThreadState state;

    void* user_stack;
    void* kernel_stack;

    trapframe_t* trapframe;
    KernelContext* kernel_context;
};

}