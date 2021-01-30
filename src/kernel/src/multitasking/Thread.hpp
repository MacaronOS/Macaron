#pragma once
#include "../hardware/trapframe.hpp"

namespace kernel::multitasking {

#define KERNEL_STACK_SIZE 1024
#define USER_STACK_SIZE 4096

enum class ThreadState {
    New,
    Running,
    Ready,
    Terminated,
    Blocked,
};

class Process;

struct kernel_context_t {
    uint32_t edi;
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
    kernel_context_t* kernel_context;
};
}