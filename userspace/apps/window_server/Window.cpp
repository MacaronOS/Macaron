#include "Window.hpp"

#include <libsys/syscalls.hpp>

#include <libgraphics/Bitmap.hpp>
#include <libgraphics/Color.hpp>

#include <wisterialib/extras.hpp>

Window::Window(uint32_t width, uint32_t height, Graphics::Bitmap&& buffer, uint32_t buffer_id, int x, int y)
    : m_width(width)
    , m_height(height)
    , m_buffer(move(buffer))
    , m_buffer_id(buffer_id)
    , m_x(x)
    , m_y(y)
{
}
