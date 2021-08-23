#pragma once

#include <Libc/Syscalls.hpp>

#include <Libsystem/Log.hpp>

#include <Libgraphics/BMP/BMPLoader.hpp>
#include <Libgraphics/Bitmap.hpp>
#include <Libgraphics/Rect.hpp>

#include <Macaronlib/Vector.hpp>
#include <Macaronlib/Runtime.hpp>

struct Click 
{
    int x, y;
};

class Mouse {
public:
    Mouse() = default;
    Mouse(int fd, int x = 0, int y = 0)
        : m_fd(fd)
        , m_x(x)
        , m_y(y)
    {
        m_cursor = Graphics::BMPLoader::load("/ext2/Resources/cursor.bmp");
        if (m_cursor.height() == 0 || m_cursor.width() == 0) {
            Log << "Could not read cursor resource" << endl;
            exit(1);
        }
    }

    inline void set_clipping(int widht, int height)
    {
        m_clipping_width = widht;
        m_clipping_height = height;
    }

    void pump();

    inline bool pressed() { return m_pressed; }
    
    inline Vector<Click> take_over_clicks() { return move(m_clicks); }

    inline int x() const { return m_x; }
    inline int y() const { return m_y; }
    inline Graphics::Rect bounds() { return Graphics::Rect(x(), y(), x() + m_cursor.width(), y() + m_cursor.height()); }

    inline int prev_x() const { return m_prev_x; }
    inline int prev_y() const { return m_prev_y; }
    inline Graphics::Rect prev_bounds() { return Graphics::Rect(prev_x(), prev_y(), prev_x() + m_cursor.width(), prev_y() + m_cursor.height()); }

    const Graphics::Bitmap& cursor() const { return m_cursor; }

private:
    int m_fd {};

    int m_clipping_width {};
    int m_clipping_height {};

    int m_x {};
    int m_y {};

    int m_prev_x {};
    int m_prev_y {};

    bool m_pressed {};
    Vector<Click> m_clicks {};

    Graphics::Bitmap m_cursor {};
};