#include "Color.hpp"

namespace Graphics {

Color::Color(uint8_t red, uint8_t green, uint8_t blue)
    : m_red(red)
    , m_green(green)
    , m_blue(blue)
{
}

Color::Color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
    : m_red(red)
    , m_green(green)
    , m_blue(blue)
    , m_alpha(alpha)
{
}

}