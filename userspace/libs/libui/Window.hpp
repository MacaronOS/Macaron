#pragma once

#include "Events.hpp"

#include <libgraphics/Bitmap.hpp>

#include <wisterialib/extras.hpp>

namespace UI {

class Window {
public:
    Window() = default;
    Window(Graphics::Bitmap&& buffer, int window_id)
        : m_buffer(move(buffer)), m_window_id(window_id)
    {
    }

    int id() const {return m_window_id;}

    int width() const { return m_width; }
    int height() const { return m_height; }

    void on_mouse_move(const UI::MouseMoveEvent& mouse_move_event) {}
    void on_mouse_click(const UI::MouseClickEvent& mouse_click_event) {}

    Graphics::Bitmap& buffer() { return m_buffer; }

private:
    Graphics::Bitmap m_buffer {};
    int m_width { 240 };
    int m_height { 180 };
    int m_window_id {};
};

}