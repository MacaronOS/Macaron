#pragma once

#include "Color.hpp"

namespace Graphics {

class Paint {
public:
    inline void set_color(const Color& color) { m_color = color; }
    const Color& color() const { return m_color; }

private:
    Color m_color;
};

}
