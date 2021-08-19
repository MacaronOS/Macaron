#pragma once

#include <Libsystem/EventLoop.hpp>

namespace UI {

enum class EventType {
    MouseMove,
    MouseClick,
    WindowResize,
    ViewRedraw,
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

struct ViewRedrawEvent {
    int left, top, right, bottom;
};

struct Event {
    EventType type;
    union {
        MouseMoveEvent mouse_move_event;
        WindowResizeEvent window_resize_event;
        MouseClickEvent mouse_click_event;
        ViewRedrawEvent view_redraw_event;
    };
};

using EventLoop = Core::EventLoop<Event>;

}
