#pragma once
#include "EventReceiver.hpp"
#include <wisterialib/Function.hpp>
#include <wisterialib/Vector.hpp>

template <typename EventType>
struct QueuedEvent {
    EventReceiver<EventType>& event_receiver;
    EventType event;
};

template <typename EventType>
class EventLoop {
public:
    void pump()
    {
        if (m_queued_events.size() > 0) {
            for (auto& queued_event : m_queued_events) {
                queued_event.event_receiver.recieve_event(queued_event.event);
            }
        }
    }

private:
    Vector<QueuedEvent<EventType>> m_queued_events {};
};