#pragma once

#include <Libc/Syscalls.hpp>

#include <Libsystem/Log.hpp>
#include <Libsystem/Syscalls.hpp>

#include <Macaronlib/ABI/Syscalls.hpp>
#include <Macaronlib/Common.hpp>
#include <Macaronlib/Function.hpp>
#include <Macaronlib/Runtime.hpp>
#include <Macaronlib/Vector.hpp>

namespace Core {

template <typename EventHolder>
struct QueuedEvent {
    Function<void(EventHolder& event)> callback;
    EventHolder event;

    void operator()()
    {
        callback(event);
    }
};

template <typename EventHolder>
class Timer {
public:
    Timer(const Function<void()>& callback, uint32_t timeout)
        : m_callback(callback)
        , m_timeout(timeout) {

        };

    bool operator()(uint32_t milliseconds_now)
    {
        if (milliseconds_now >= m_last_invokation + m_timeout) {
            m_callback();
            m_last_invokation = milliseconds_now;
            return true;
        }
        return false;
    }

private:
    Function<void()> m_callback {};
    uint32_t m_timeout {};
    uint32_t m_last_invokation {};
};

class FDSelector {
public:
    FDSelector(const Function<void()>& callback, uint8_t fd)
        : m_callback(callback)
        , m_fd(fd)
    {
    }
    bool operator()(fd_set* fds)
    {
        if (FD_IS_SET(fds, m_fd)) {
            m_callback();
            return true;
        }
        return false;
    }

private:
    Function<void()> m_callback {};
    uint8_t m_fd {};
};

template <typename EventHolder>
class EventLoop {
public:
    static inline EventLoop<EventHolder>& the()
    {
        return s_the;
    }

    void register_timer(const Function<void()>& callback, uint32_t milliseconds)
    {
        m_timers.push_back(Timer<EventHolder>(callback, milliseconds));
    }

    void register_fd_for_select(const Function<void()>& callback, uint8_t fd)
    {
        m_fd_selectors.push_back(FDSelector(callback, fd));
        m_nfds = max(m_nfds, fd + 1);
    }

    void enqueue_callback_for_event(const Function<void(const EventHolder& event)>& callback, const EventHolder& event)
    {
        m_event_holders.push_back({ callback, event });
    }

    void enqueue_callback(const Function<void()>& callback)
    {
        m_callbacks.push_back(callback);
    }

    inline void run()
    {
        while (true) {
            pump();
        }
    }

    void pump()
    {
        bool processed = false;
        processed |= process_callbasks();
        processed |= process_timers();
        processed |= process_fd_selectors();
        processed |= process_event_holders();

        if (!processed) {
            sched_yield();
        }
    }

    bool process_timers()
    {
        bool processed = false;

        timespec ts;
        clock_gettime(CLOCK_MONOTONIC_COARSE, &ts);
        uint32_t milliseconds_now = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;

        for (auto& timer : m_timers) {
            processed |= timer(milliseconds_now);
        }

        return processed;
    }

    bool process_fd_selectors()
    {
        bool processed = false;

        fd_set read_fds;
        select(m_nfds, &read_fds, nullptr, nullptr, nullptr);

        for (auto& fd_selector : m_fd_selectors) {
            processed |= fd_selector(&read_fds);
        }

        return processed;
    }

    bool process_event_holders()
    {
        if (m_event_holders.size() == 0) {
            return false;
        }
        for (auto& event_holder : m_event_holders) {
            event_holder();
        }
        m_event_holders.clear();
        return true;
    }

    bool process_callbasks()
    {
        if (m_callbacks.size() == 0) {
            return false;
        }
        for (auto& callback : m_callbacks) {
            callback();
        }
        m_callbacks.clear();
        return true;
    }

public:
    static EventLoop<EventHolder> s_the;

    Vector<Timer<EventHolder>> m_timers {};
    Vector<QueuedEvent<EventHolder>> m_event_holders {};
    Vector<Function<void()>> m_callbacks {};
    Vector<FDSelector> m_fd_selectors {};
    int m_nfds {};
};

template <typename Event>
EventLoop<Event> EventLoop<Event>::s_the;

}