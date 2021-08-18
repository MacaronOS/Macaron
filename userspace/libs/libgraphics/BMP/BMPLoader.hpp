#pragma once

#include "../Bitmap.hpp"

#include <wisterialib/String.hpp>

namespace Graphics::BMPLoader {

Bitmap load(const String& path);

}