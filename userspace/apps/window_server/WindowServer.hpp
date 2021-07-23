#pragma once

#include "Screen.hpp"
#include "Window.hpp"
#include "Mouse.hpp"

#include <wisterialib/ObjectPool.hpp>
#include <wisterialib/List.hpp>

#include <libgraphics/ws/Connection.hpp>
#include <libgraphics/Rect.hpp>

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

    Window* get_window_by_id(int id);

private:
    Screen m_screen {};
    Mouse m_mouse {};
    WS::ServerConnection m_connection {};
    List<Window*> m_windows {};
    Vector<Graphics::Rect> m_invalid_areas {};
};