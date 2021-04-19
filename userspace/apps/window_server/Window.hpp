#pragma once

#include <libgraphics/Bitmap.hpp>
#include <libgraphics/Rect.hpp>

class Window {
public:
    Window(uint32_t width, uint32_t height, Graphics::Bitmap&& buffer, uint32_t buffer_id, int x, int y);

    Graphics::Bitmap& buffer() { return m_buffer; }

    uint32_t width() const { return m_width; }
    uint32_t height() const { return m_height; }

    int x() const { return m_x; }
    int y() const { return m_y; }

private:
    uint32_t m_width {};
    uint32_t m_height {};

    int m_x {};
    int m_y {};

    uint32_t m_buffer_id {};
    Graphics::Bitmap m_buffer {};
};
