#pragma once

#include <wisterialib/common.hpp>

namespace Graphics {

class Color {
public:
    Color() = default;
    Color(uint8_t red, uint8_t green, uint8_t blue);

    Color(const Color&) = default;
    Color& operator=(const Color& color) = default;

    inline uint8_t red() const { return m_red; };
    inline uint8_t green() const { return m_green; };
    inline uint8_t blue() const { return m_blue; };

    inline void set_red(uint8_t red) { m_red = red; }
    inline void set_green(uint8_t green) { m_green = green; }
    inline void set_blue(uint8_t blue) { m_blue = blue; }

private:
    uint8_t m_blue {};
    uint8_t m_green {};
    uint8_t m_red {};
    uint8_t padding {};
};

}
