#include "Window.hpp"
#include "Resources.hpp"

#include <Libsystem/Syscalls.hpp>

#include <Libgraphics/BMP/BMPLoader.hpp>
#include <Libgraphics/Bitmap.hpp>
#include <Libgraphics/Canvas.hpp>
#include <Libgraphics/Color.hpp>

#include <Macaronlib/Runtime.hpp>

static uint32_t window_id = 0;
uint32_t next_window_id()
{
    return window_id++;
}

Window::Window(const String& titile, uint32_t width, uint32_t height, Graphics::Bitmap&& buffer, uint32_t buffer_id, int pid, int x, int y)
    : m_width(width)
    , m_height(height)
    , m_buffer(move(buffer))
    , m_buffer_id(buffer_id)
    , m_x(x)
    , m_y(y)
    , m_pid(pid)
    , id(next_window_id())
{
    m_frame_buffer = Graphics::Bitmap((Graphics::Color*)malloc(4 * width * frame_height), width, frame_height);

    for (int y = 0; y < frame_height; y++) {
        for (int x = 0; x < width; x++) {
            m_frame_buffer[y][x] = Graphics::Color(255, 255, 255);
        }
    }

    auto close_offset = m_width - 6 - Resources::the().maximize_button().width();
    auto minimize_offset = close_offset - 20;
    auto maximize_offset = minimize_offset - 20;
    auto back_offset = maximize_offset - 25;

    auto canvas = Graphics::Canvas(m_frame_buffer);
    canvas.draw_text(titile, 8, 3, Resources::the().font_bold());
    canvas.draw_bitmap(Resources::the().maximize_button(), maximize_offset, 4);
    canvas.draw_bitmap(Resources::the().minimize_button(), minimize_offset, 4);
    canvas.draw_bitmap(Resources::the().close_button(), close_offset, 4);
    canvas.draw_bitmap(Resources::the().back_button(), back_offset, 4);

    m_close_button_position = Graphics::Rect(
        close_offset,
        4,
        close_offset + Resources::the().close_button().width(),
        4 + Resources::the().close_button().height());

    m_minimize_button_position = Graphics::Rect(
        minimize_offset,
        4,
        minimize_offset + Resources::the().minimize_button().width(),
        4 + Resources::the().minimize_button().height());

    m_maximize_button_position = Graphics::Rect(
        maximize_offset,
        4,
        maximize_offset + Resources::the().maximize_button().width(),
        4 + Resources::the().maximize_button().height());

    m_back_button_position = Graphics::Rect(
        back_offset,
        4,
        back_offset + Resources::the().back_button().width(),
        4 + Resources::the().back_button().height());
}
