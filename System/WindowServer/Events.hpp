#pragma once

#include <Libsystem/EventLoop.hpp>

enum class EventType {
    MouseMove,
    MouseResize,
};

struct MouseMoveEvent {
    int x, y;
};

struct MouseResizeEvent {
    int width, height;
};

struct Event {
    EventType type;
    union {
        MouseMoveEvent move_event;
        MouseResizeEvent resize_event;
    };
};

using EventLoop = Core::EventLoop<Event>;
