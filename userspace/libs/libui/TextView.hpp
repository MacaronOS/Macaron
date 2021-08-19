#pragma once

#include "Gravity.hpp"
#include "View.hpp"
#include <libgraphics/Font/BitmapFont.hpp>

namespace UI {

class TextView : public View {
public:
    void on_measure(int width_measure_spec, int height_measure_spec) override;
    void on_draw(Graphics::Canvas& canvas) override;

    inline void set_text(const String& text) { m_text = text; }
    inline void set_text_color(const Graphics::Color& color) { m_color = color; }
    // TODO: typeface instead of bitmap font
    inline void set_typeface(Graphics::BitmapFont* font) { m_font = font; }
    inline void set_gravity(Gravity gravity) { m_gravity = gravity; }

protected:
    inline void set_measured_text_dimensions(int width, int height)
    {
        m_measured_text_width = width;
        m_measured_text_height = height;
    }

protected:
    String m_text {};
    Graphics::Color m_color {};
    Graphics::BitmapFont* m_font {};
    Gravity m_gravity {};
    int m_measured_text_width {};
    int m_measured_text_height {};
};

}
