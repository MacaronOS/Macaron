#pragma once

#include <Libgraphics/Bitmap.hpp>
#include <Libgraphics/Rect.hpp>

class Window {
public:
    Window(uint32_t width, uint32_t height, Graphics::Bitmap&& buffer, uint32_t buffer_id, int pid, int x, int y);

    uint32_t id;

    int pid() const { return m_pid; }

    Graphics::Bitmap& buffer() { return m_buffer; }
    Graphics::Bitmap& frame_buffer() { return m_frame_buffer; }

    uint32_t width() const { return m_width; }
    uint32_t height() const { return m_height; }

    int x() const { return m_x; }
    int y() const { return m_y; }

    inline Graphics::Rect bounds() { return Graphics::Rect(x(), y(), x() + width(), y() + height()); }
    inline Graphics::Rect frame_bounds() { return Graphics::Rect(x(), y() - frame_height, x() + width(), y()); }
    inline Graphics::Rect all_bounds() { return Graphics::Rect(x(), y() - frame_height, x() + width(), y() + height()); }

    void move_position(int del_x, int del_y)
    {
        m_x += del_x;
        m_y += del_y;
    }

private:
    static constexpr int frame_height = 24;
    int m_pid;
    uint32_t m_width {};
    uint32_t m_height {};

    int m_x {};
    int m_y {};

    uint32_t m_buffer_id {};
    Graphics::Bitmap m_buffer {};

    Graphics::Bitmap m_frame_buffer;
};
