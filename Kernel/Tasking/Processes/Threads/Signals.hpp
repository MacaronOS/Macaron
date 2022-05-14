#pragma once

#include <Macaronlib/ABI/Signals.hpp>
#include <Macaronlib/Callback.hpp>
#include <Macaronlib/Common.hpp>
#include <Tasking/MemoryDescription/SharedVMArea.hpp>

namespace Kernel::Tasking {

class Signals {
public:
    // Signal mask.
    // Used to block signals.
    inline void mask_block(uint32_t value)
    {
        m_mask &= ~value;
    }

    inline void mask_unblock(uint32_t value)
    {
        m_mask |= value;
    }

    inline void mask_set(uint32_t value)
    {
        m_mask = value;
    }

    inline uint32_t mask() const { return m_mask; }

    // Pending mask.
    // Used to dispatch signals.
    inline void add_pending(int signo)
    {
        m_pending_mask |= (1 << signo);
    }

    inline void remove_pending(int signo)
    {
        m_pending_mask &= ~((uint32_t)(1 << signo));
    }

    inline bool is_pending(int signo)
    {
        return m_pending_mask & m_mask & (1 << signo);
    }

    inline uint32_t pending_mask() const { return m_pending_mask; }

    // Signal handlers.
    void set_handler(int signo, void* handler) { m_handlers[signo] = handler; }
    void* handler(int signo) { return m_handlers[signo]; }

    template <typename Callback>
    void dispatch_pending(Callback callback)
    {
        for (int signo = 1; signo < NSIG; signo++) {
            if (is_pending(signo)) {
                remove_pending(signo);
                if (callback(signo) == CallbackIterate::Stop) {
                    break;
                }
            }
        }
    }

    static void setup_caller();
    static uintptr_t caller_ip();

private:
    uint32_t m_mask { 0xffffffff }; // all signals are alloved
    uint32_t m_pending_mask {};
    void* m_handlers[NSIG] {};
};

}