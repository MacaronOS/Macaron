#pragma once

#include <libgraphics/Bitmap.hpp>
#include <wisterialib/String.hpp>

namespace BMPLoader {

Graphics::Bitmap load(const String& path);

}