#pragma once

#include "Screen.hpp"
#include "Window.hpp"
#include "Mouse.hpp"
#include "Events.hpp"
#include "Connection.hpp"

#include "Font/FontLoader.hpp"

#include <wisterialib/ObjectPool.hpp>
#include <wisterialib/List.hpp>

#include <libgraphics/Rect.hpp>


class WindowServer {
    static constexpr auto windows = 10;
public:
    WindowServer() : m_event_loop(EventLoop::the()) {}
    
    bool initialize();
    void run();

    void process_message(WS::WSProtocol& message);

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
    Connection m_connection {Connection([this](WS::WSProtocol& message) {process_message(message);})};
    List<Window*> m_windows {};
    Vector<Graphics::Rect> m_invalid_areas {};
    EventLoop& m_event_loop;
    bool m_mouse_needs_draw_since_moved { true };
};