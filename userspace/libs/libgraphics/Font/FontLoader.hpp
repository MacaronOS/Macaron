#pragma once

#include "BitmapFont.hpp"

#include <libgraphics/Bitmap.hpp>
#include <wisterialib/String.hpp>
#include <wisterialib/memory.hpp>

namespace Graphics::FontLoader {

BitmapFont load(const String& text_description_path, const String& font_texture_path);

}