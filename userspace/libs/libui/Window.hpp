#pragma once

#include "Events.hpp"
#include "View.hpp"

#include <libgraphics/Bitmap.hpp>

#include <wisterialib/extras.hpp>

namespace UI {

class Application;

class Window {
    friend class Application;

public:
    Window() = default;
    Window(Graphics::Bitmap&& buffer, int window_id, int widht, int height)
        : m_buffer(move(buffer))
        , m_window_id(window_id)
        , m_width(widht)
        , m_height(height)
    {
    }

    // properties
    inline int id() const { return m_window_id; }
    inline int width() const { return m_width; }
    inline int height() const { return m_height; }

    // children
    inline void set_content_view(View* content_view)
    {
        m_content_view = content_view;
        m_content_view->request_layout();
    }
    inline View* content_view() { return m_content_view; }

    void on_mouse_move(const UI::MouseMoveEvent& mouse_move_event) { }
    void on_mouse_click(const UI::MouseClickEvent& mouse_click_event) { }

    Graphics::Bitmap& buffer() { return m_buffer; }

private:
    Graphics::Bitmap m_buffer {};
    int m_width {};
    int m_height {};
    int m_window_id {};
    View* m_content_view {};
};

}