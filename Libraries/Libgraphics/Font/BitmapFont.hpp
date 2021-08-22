#pragma once

#include "../Bitmap.hpp"

#include <Wisterialib/String.hpp>
#include <Wisterialib/Common.hpp>

namespace Graphics {

struct CharDescriptor {
    uint16_t x {};
    uint16_t y {};
    uint16_t width {};
    uint16_t height {};
    int16_t xoffset {};
    int16_t yoffset {};
    int16_t xadvantage {};
};

struct BitmapFont {
    String name {};
    uint16_t line_height {};
    uint16_t base {};
    int16_t width {};
    uint16_t height {};
    CharDescriptor chars[256] {};
    int16_t kerning[256][256] {};
    Bitmap texture {};
};

}