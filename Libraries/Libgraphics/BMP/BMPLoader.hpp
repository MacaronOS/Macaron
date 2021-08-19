#pragma once

#include "../Bitmap.hpp"

#include <Wisterialib/String.hpp>

namespace Graphics::BMPLoader {

Bitmap load(const String& path);

}