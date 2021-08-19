#pragma once

#include "BitmapFont.hpp"

#include <Libgraphics/Bitmap.hpp>
#include <Wisterialib/String.hpp>
#include <Wisterialib/memory.hpp>

namespace Graphics::FontLoader {

BitmapFont load(const String& text_description_path, const String& font_texture_path);

}