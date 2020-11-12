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

struct Thread {
    Process* process;
    ThreadState state;

    void* user_stack;
    void* kernel_stack;

    trapframe_t* trapframe;
};
}