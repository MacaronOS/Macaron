#pragma once

#include <Libsystem/EventLoop.hpp>
#include <Macaronlib/ABI/Keyboard.hpp>

namespace UI {

enum class EventType {
    MouseMove,
    MouseClick,
    WindowResize,
    ViewRedraw,
    Keyboard,
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

using KeyboardEvent = KeyboardPacket;

struct Event {
    EventType type;
    union {
        MouseMoveEvent mouse_move_event;
        WindowResizeEvent window_resize_event;
        MouseClickEvent mouse_click_event;
        ViewRedrawEvent view_redraw_event;
        KeyboardEvent keyboard_event;
    };
};

using EventLoop = Core::EventLoop<Event>;

}
