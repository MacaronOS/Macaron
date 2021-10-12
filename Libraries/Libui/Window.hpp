#pragma once

#include "Events.hpp"
#include "View.hpp"

#include <Libgraphics/Bitmap.hpp>

#include <Macaronlib/Runtime.hpp>

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

    inline int id() const { return m_window_id; }
    inline int width() const { return m_width; }
    inline int height() const { return m_height; }

    Graphics::Bitmap& buffer() { return m_buffer; }

private:
    Graphics::Bitmap m_buffer {};
    int m_width {};
    int m_height {};
    int m_window_id {};
};

}