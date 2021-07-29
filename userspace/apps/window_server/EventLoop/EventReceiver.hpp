#pragma once

template <typename EventType>
struct EventReceiver {
    virtual ~EventReceiver() = default;
    virtual void recieve_event(EventType& event);
};