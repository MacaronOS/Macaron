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

    inline Color* colors() { return m_colors; }

    inline Color* operator[](size_t index) { return m_colors + index * m_width; }

    inline uint32_t width() const { return m_width; }
    inline uint32_t height() const { return m_height; }

private:
    Color* m_colors {};
    uint32_t m_width {};
    uint32_t m_height {};
};

}
