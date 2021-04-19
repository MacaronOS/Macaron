#pragma once

#include "Bitmap.hpp"

#include <wisterialib/extras.hpp>

namespace Graphics {

class Window {
public:
    Window() = default;
    Window(Graphics::Bitmap&& buffer)
        : m_buffer(move(buffer))
    {
    }

    int width() const { return m_width; }
    int height() const { return m_height; }

    Graphics::Bitmap& buffer() { return m_buffer; }

private:
    Graphics::Bitmap m_buffer {};
    int m_width { 240 };
    int m_height { 180 };
};

}