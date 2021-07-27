#pragma once

#include <libgraphics/Bitmap.hpp>
#include <wisterialib/String.hpp>
#include <wisterialib/memory.hpp>

struct CharDescriptor {
    uint16_t x {};
    uint16_t y {};
    uint16_t width {};
    uint16_t height {};
    int16_t xoffset {};
    int16_t yoffset {};
    int16_t xadvantage {};
};

struct Font {
    String name {};
    uint16_t line_height {};
    uint16_t base {};
    int16_t width {};
    uint16_t height {};
    CharDescriptor chars[256] {};
    int16_t kerning[256][256] {};
    Graphics::Bitmap texture {};
};

namespace FontLoader {

Font load(const String& text_description_path, const String& font_texture_path);

}