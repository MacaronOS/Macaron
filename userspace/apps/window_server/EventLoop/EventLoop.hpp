#pragma once
#include "EventReceiver.hpp"
#include <wisterialib/Function.hpp>
#include <wisterialib/Vector.hpp>
#include <libc/syscalls.hpp>
#include <libsys/Log.hpp>
#include <libsys/syscalls.hpp>
#include <wisterialib/extras.hpp>
#include <wisterialib/posix/defines.hpp>

namespace Core {

template <typename EventHolder>
struct QueuedEvent {
    const Function<int(EventHolder& event)>& callback;
    EventHolder event;

    int operator() () {
        callback(event);
        return true;
    }
};

template <typename EventHolder>
class Timer {
public:
    Timer(const Function<void()>& callback, uint32_t timeout) : m_callback(callback), m_timeout(timeout)
    {
        
    };

    bool operator() (uint32_t milliseconds_now) {
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
    FDSelector(const Function<void()>& callback, uint8_t fd) : m_callback(callback), m_fd(fd) {}
    bool operator() (fd_set* fds) {
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
    static inline EventLoop<EventHolder>& the() {
        return s_the;
    }

    void register_timer(const Function<void()>& callback, uint32_t milliseconds) {
        m_timers.push_back(Timer<EventHolder>(callback, milliseconds));
    }

    void register_fd_for_select(const Function<void()>& callback, uint8_t fd)
    {
        m_fd_selectors.push_back(FDSelector(callback, fd));
        m_nfds = max(m_nfds, fd + 1);
    }

    void pump()
    {
        bool processed = false;
        processed |= process_timers();
        processed |= process_fd_selectors();

        if (!processed) {
            // TODO: yield here
        }
    }

    bool process_timers() {
        bool processed = false;

        timespec ts;
        clock_gettime(CLOCK_MONOTONIC_COARSE, &ts);
        uint32_t milliseconds_now = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;

        for (size_t at = 0 ; at < m_timers.size() ; at++) {
            processed |= m_timers[at](milliseconds_now);
        }

        return processed;
    }

    bool process_fd_selectors() {
        bool processed = false;

        fd_set read_fds;
        select(m_nfds, &read_fds, nullptr, nullptr, nullptr);

        for (size_t at = 0 ; at < m_fd_selectors.size() ; at++) {
            processed |= m_fd_selectors[at](&read_fds);
        }

        return processed;
    }

public:
    static EventLoop<EventHolder> s_the;

    Vector<Timer<EventHolder>> m_timers {};
    Vector<QueuedEvent<EventHolder>> m_event_holders {};
    Vector<FDSelector> m_fd_selectors {};
    int m_nfds {};
};

template<typename Event>
EventLoop<Event> EventLoop<Event>::s_the;

}