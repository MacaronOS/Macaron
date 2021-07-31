#pragma once

#include "Screen.hpp"
#include "Window.hpp"
#include "Mouse.hpp"

#include "Font/FontLoader.hpp"

#include "EventLoop/EventLoop.hpp"

#include <wisterialib/ObjectPool.hpp>
#include <wisterialib/List.hpp>

#include <libgraphics/ws/Connection.hpp>
#include <libgraphics/Rect.hpp>

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

class WindowServer {
    static constexpr auto windows = 10;
public:
    WindowServer() = default;
    
    bool initialize();
    void run();

private:
    void redraw();
    void draw_windows();
    void draw_mouse();
    void draw_background();
    void copy_changes_to_second_buffer();

    void draw_text(const String& text, int x, int y, const Font& font);

    Window* get_window_by_id(int id);

private:
    Screen m_screen {};
    Graphics::Bitmap m_wallpaper {};
    Graphics::Bitmap m_cursor {};
    Font m_font_medium {};
    Font m_font_bold {};
    Mouse m_mouse {};
    WS::ServerConnection m_connection {};
    List<Window*> m_windows {};
    Vector<Graphics::Rect> m_invalid_areas {};
    EventLoop<Event> m_event_loop {};
};