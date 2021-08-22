#pragma once

#include <Macaronlib/Common.hpp>

namespace Graphics {

class Color {
public:
    Color() = default;
    Color(uint8_t red, uint8_t green, uint8_t blue);
    Color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha);

    Color(const Color&) = default;
    Color& operator=(const Color& color) = default;

    inline uint8_t red() const { return m_red; };
    inline uint8_t green() const { return m_green; };
    inline uint8_t blue() const { return m_blue; };
    inline uint8_t alpha() const { return m_alpha; };

    inline void set_red(uint8_t red) { m_red = red; }
    inline void set_green(uint8_t green) { m_green = green; }
    inline void set_blue(uint8_t blue) { m_blue = blue; }
    inline void set_alpha(uint8_t alpha) { m_alpha = alpha; }

    inline void mix_with(const Color& clr)
    {
        if (clr.alpha() == 0) {
            return;
        }

        if (clr.alpha() == 255) {
            *this = clr;
            return;
        }

        int alpha_c = 255 * (alpha() + clr.alpha()) - alpha() * clr.alpha();
        int alpha_of_me = alpha() * (255 - clr.alpha());
        int alpha_of_it = 255 * clr.alpha();

        m_red = (red() * alpha_of_me + clr.red() * alpha_of_it) / alpha_c;
        m_green = (green() * alpha_of_me + clr.green() * alpha_of_it) / alpha_c;
        m_blue = (blue() * alpha_of_me + clr.blue() * alpha_of_it) / alpha_c;
        m_alpha = alpha_c / 255;
    }

private:
    uint8_t m_blue {};
    uint8_t m_green {};
    uint8_t m_red {};
    uint8_t m_alpha { 255 };
};

}
