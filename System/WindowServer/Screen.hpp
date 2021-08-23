#pragma once
#include <Libgraphics/Bitmap.hpp>
#include <Libgraphics/Rect.hpp>

class Screen {
    static constexpr uint32_t width = 1024;
    static constexpr uint32_t height = 768;

public:
    Screen() = default;
    Screen(uint8_t screen_fd, Graphics::Bitmap&& front_buffer, Graphics::Bitmap&& back_buffer);

    Screen(const Screen&) = delete;
    Screen& operator=(const Screen&) = delete;

    Screen(Screen&&);
    Screen& operator=(Screen&&);

    Graphics::Bitmap& front_buffer();
    const Graphics::Bitmap& front_buffer() const;

    Graphics::Bitmap& back_buffer();
    const Graphics::Bitmap& back_buffer() const;

    void swap_buffers();

    const Graphics::Rect& bounds() const { return m_bounds; }

private:
    uint8_t m_screen_fd {};
    Graphics::Bitmap m_first_buffer {};
    Graphics::Bitmap m_second_buffer {};

    Graphics::Bitmap* m_front_buffer {};
    Graphics::Bitmap* m_back_buffer {};

    Graphics::Rect m_bounds { Graphics::Rect(0, 0, width, height) };
};