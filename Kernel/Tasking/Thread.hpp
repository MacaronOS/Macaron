#pragma once

#include <Hardware/DescriptorTables/GDT.hpp>
#include <Hardware/Trapframe.hpp>
#include <Macaronlib/ABI/Signals.hpp>
#include <Memory/Region.hpp>

namespace Kernel::Tasking {

using namespace Memory;

#define KERNEL_STACK_SIZE 4096
#define USER_STACK_SIZE 512 * 4096

enum class ThreadState {
    New,
    Running,
    Ready,
    Terminated,
    Blocked,
};

// Not realy used inside the C++ code.
// Just to clarify the sequence of push / pop opertions inside Scheduling.s block routines.
struct KernelContext {
    uint32_t ebx;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
};

class Process;

class Thread {
    friend class Scheduler;
    friend class Process;

public:
    enum class Privilege {
        Kernel,
        User,
    };

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

    ThreadState state() const { return m_state; }
    uint32_t kernel_stack() const { return m_kernel_stack; }
    uint32_t kernel_stack_top() const { return m_kernel_stack + KERNEL_STACK_SIZE; }
    uint32_t user_stack() const { return m_user_stack; }
    uint32_t user_stack_top() const { return m_user_stack + USER_STACK_SIZE; }
    void* user_stack_ptr() const { return (void*)m_user_stack; }
    Trapframe* trapframe() const { return (Trapframe*)(m_kernel_stack + KERNEL_STACK_SIZE - sizeof(Trapframe)); }
    KernelContext** kernel_context() { return &m_kernel_context; }
    bool blocked_in_kernel()
    {
        return m_kernel_context != nullptr;
    }

    inline void Terminate() { m_state = ThreadState::Terminated; }

    inline void set_privilege(Privilege pr)
    {
        m_privilege = pr;
        if (pr == Privilege::User) {
            constexpr uint32_t data = Kernel::DescriptorTables::GDT::UserDataOffset | Kernel::DescriptorTables::GDT::RequestRing3;
            constexpr uint32_t code = Kernel::DescriptorTables::GDT::UserCodeOffset | Kernel::DescriptorTables::GDT::RequestRing3;
            trapframe()->ds = data;
            trapframe()->es = data;
            trapframe()->fs = data;
            trapframe()->gs = data;
            trapframe()->cs = code;
            trapframe()->ss = data;
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

    // Block signals
    inline void signal_mask_block(uint32_t value)
    {
        m_signal_mask &= ~value;
    }

    inline void signal_mask_unblock(uint32_t value)
    {
        m_signal_mask |= value;
    }

    inline void signal_mask_set(uint32_t value)
    {
        m_signal_mask = value;
    }

    inline uint32_t signal_mask() const { return m_signal_mask; }

    // Dispatch signals
    inline void signal_add_pending(int signo)
    {
        m_pending_signals_mask |= (1 << signo);
    }

    inline void signal_remove_pending(int signo)
    {
        m_pending_signals_mask &= ~((uint32_t)(1 << signo));
    }

    inline bool signal_is_pending(int signo)
    {
        return m_pending_signals_mask & m_signal_mask & (1 << signo);
    }

    inline uint32_t pending_singal_mask() const { return m_pending_signals_mask; }

    // Signal handlers
    void set_signal_handler(int signo, void* handler) { m_signal_handlers[signo] = handler; }
    void* signal_handler(int signo) { return m_signal_handlers[signo]; }
    void jump_to_signal_caller(int signo);


private:
    Process* m_process;
    ThreadState m_state { ThreadState::Ready };
    Privilege m_privilege { Privilege::User };

    uint32_t m_user_stack {};
    uint32_t m_kernel_stack {};

    KernelContext* m_kernel_context {};

    uint32_t m_signal_mask { 0xffffffff }; // all signals are alloved
    uint32_t m_pending_signals_mask {};
    void* m_signal_handlers[NSIG] {};
};

}