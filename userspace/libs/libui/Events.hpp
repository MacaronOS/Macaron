#pragma once

#include <libsys/EventLoop.hpp>

namespace UI {

enum class EventType {
    MouseMove,
    WindowResize,
};

struct MouseMoveEvent {
    int x, y;
};

struct MouseClickEvent {
    int x, y;
};

struct WindowResizeEvent {
    int width, height;
};

struct Event {
    EventType type;
    union {
        MouseMoveEvent mouse_move_event;
        WindowResizeEvent window_resize_event;
        MouseClickEvent mouse_click_event;
    };
};

using EventLoop = Core::EventLoop<Event>;

}
