#pragma once

#include "Color.hpp"

#include <wisterialib/common.hpp>

namespace Graphics {

class Bitmap {
public:
    Bitmap() = default;
    Bitmap(Color* colors, size_t width, size_t height);

    Bitmap(const Bitmap&) = delete;
    Bitmap& operator=(const Bitmap&) = delete;

    Bitmap(Bitmap&& bitmap);
    Bitmap& operator=(Bitmap&&);

    Color* colors() { return m_colors; }

    Color* operator[](size_t index) { return m_colors + index * m_width; }

private:
    Color* m_colors {};
    uint32_t m_width {};
};

}
