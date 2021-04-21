#pragma once

#include "Screen.hpp"
#include "Window.hpp"
#include "Mouse.hpp"

#include <wisterialib/ObjectPool.hpp>
#include <wisterialib/List.hpp>

#include <libgraphics/ws/Connection.hpp>

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

private:
    Screen m_screen {};
    Mouse m_mouse {};
    WS::ServerConnection m_connection {};
    List<Window*> m_windows {};
};