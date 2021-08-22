#pragma once

#include "../Bitmap.hpp"

#include <Macaronlib/String.hpp>

namespace Graphics::BMPLoader {

Bitmap load(const String& path);

}