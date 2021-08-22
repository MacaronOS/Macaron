#include "Screen.hpp"

#include <Libc/Syscalls.hpp>

#include <Libsystem/Log.hpp>

#include <Wisterialib/ABI/Syscalls.hpp>
#include <Wisterialib/Runtime.hpp>

Screen::Screen(uint8_t screen_fd, Graphics::Bitmap&& first_buffer, Graphics::Bitmap&& second_buffer)
    : m_screen_fd(screen_fd)
    , m_first_buffer(move(first_buffer))
    , m_second_buffer(move(second_buffer))
    , m_front_buffer(&m_first_buffer)
    , m_back_buffer(&m_second_buffer)
{
}

Screen::Screen(Screen&& screen)
{
    *this = move(screen);
}

Screen& Screen::operator=(Screen&& screen)
{
    m_screen_fd = screen.m_screen_fd;

    m_first_buffer = move(screen.m_first_buffer);
    m_second_buffer = move(screen.m_second_buffer);

    if (screen.m_front_buffer == &screen.m_first_buffer) {
        m_front_buffer = &m_first_buffer;
        m_back_buffer = &m_second_buffer;
    } else {
        m_front_buffer = &m_second_buffer;
        m_back_buffer = &m_first_buffer;
    }

    screen.m_front_buffer = nullptr;
    screen.m_back_buffer = nullptr;

    return *this;
}

Graphics::Bitmap& Screen::front_buffer()
{
    return *m_front_buffer;
}

const Graphics::Bitmap& Screen::front_buffer() const
{
    return *m_front_buffer;
}

Graphics::Bitmap& Screen::back_buffer()
{
    return *m_back_buffer;
}

const Graphics::Bitmap& Screen::back_buffer() const
{
    return *m_back_buffer;
}

void Screen::swap_buffers()
{
    auto copy_front = m_front_buffer;
    m_front_buffer = m_back_buffer;
    m_back_buffer = copy_front;

    ioctl(m_screen_fd, BGA_SWAP_BUFFERS);
}
