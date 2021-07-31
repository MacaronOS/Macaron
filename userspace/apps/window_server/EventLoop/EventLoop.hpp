#pragma once
#include "EventReceiver.hpp"
#include <wisterialib/Function.hpp>
#include <wisterialib/Vector.hpp>
#include <libc/syscalls.hpp>
#include <libsys/Log.hpp>
#include <libsys/syscalls.hpp>
#include <wisterialib/extras.hpp>
#include <wisterialib/posix/defines.hpp>

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

template <typename EventHolder>
class EventLoop {
public:
    void register_timer(const Function<void()>& callback, uint32_t milliseconds) {
        m_timers.push_back(Timer<EventHolder>(callback, milliseconds));
    }

    void pump()
    {
        bool processed = false;

        timespec ts;
        clock_gettime(CLOCK_MONOTONIC_COARSE, &ts);
        uint32_t milliseconds_now = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;

        for (size_t at = 0 ; at < m_timers.size() ; at++) {
            processed |= m_timers[at](milliseconds_now);
        }

        if (!processed) {
            // TODO: yield here
        }
    }

private:
    Vector<Timer<EventHolder>> m_timers {};
    Vector<QueuedEvent<EventHolder>> m_event_holders {};
};