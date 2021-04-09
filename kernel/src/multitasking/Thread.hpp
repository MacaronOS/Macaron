#pragma once
#include "../hardware/trapframe.hpp"

#include <hardware/descriptor_tables.hpp>
#include <memory/Region.hpp>
#include <memory/vmm.hpp>

namespace kernel::multitasking {

using namespace memory;

#define KERNEL_STACK_SIZE 4096
#define USER_STACK_SIZE 4096

enum class ThreadState {
    New,
    Running,
    Ready,
    Terminated,
    Blocked,
};

struct KernelContext {
    uint32_t edi; // eax, ecx,
    uint32_t esi;
    uint32_t ebx;
    uint32_t ebp;
    uint32_t esp;
    uint32_t eip;
};

class Process;

class Thread {
    friend class TaskManager;
    friend class Process;

public:
    enum class Privilege {
        Kernel,
        User,
    };

public:
    explicit Thread(Process* proc, Privilege pr = Privilege::User)
        : m_process(proc)
    {
        setup(pr);
    }

    ~Thread()
    {
        free((void*)m_kernel_stack);
    }

    void setup(Privilege pr = Privilege::User);
    static Thread* TieNewTo(Process* proc);

public:
    ThreadState state() const { return m_state; }
    uint32_t kernel_stack() const { return m_kernel_stack; }
    uint32_t kernel_stack_top() const { return m_kernel_stack + KERNEL_STACK_SIZE; }
    uint32_t user_stack() const { return m_user_stack; }
    uint32_t user_stack_top() const { return m_user_stack + USER_STACK_SIZE; }
    void* user_stack_ptr() const { return (void*)m_user_stack; }
    trapframe_t* trapframe() const { return (trapframe_t*)(m_kernel_stack + KERNEL_STACK_SIZE - sizeof(trapframe_t)); }

public:
    inline void Terminate() { m_state = ThreadState::Terminated; }

    inline void set_privilege(Privilege pr)
    {
        m_privilege = pr;
        if (pr == Privilege::User) {
            trapframe()->ds = GDT_USER_DATA_OFFSET | REQUEST_RING_3;
            trapframe()->es = GDT_USER_DATA_OFFSET | REQUEST_RING_3;
            trapframe()->fs = GDT_USER_DATA_OFFSET | REQUEST_RING_3;
            trapframe()->gs = GDT_USER_DATA_OFFSET | REQUEST_RING_3;
            trapframe()->cs = GDT_USER_CODE_OFFSET | REQUEST_RING_3;
            trapframe()->ss = GDT_USER_DATA_OFFSET | REQUEST_RING_3;
        }
        // TODO: support kernel threads
    }

    inline void reset_stack()
    {
        trapframe()->useresp = user_stack_top();
        trapframe()->ebp = user_stack_top();
    }

    inline void reset_flags()
    {
        trapframe()->eflags = 0x202;
    }

private:
    Process* m_process;
    ThreadState m_state { ThreadState::Ready };
    Privilege m_privilege { Privilege::User };

    uint32_t m_user_stack {};
    uint32_t m_kernel_stack {};

    KernelContext* m_kernel_context {};
};

}