#pragma once

#include "Gravity.hpp"
#include "View.hpp"
#include <Libgraphics/Font/BitmapFont.hpp>

namespace UI {

class TextView : public View {
public:
    void on_measure(int width_measure_spec, int height_measure_spec) override;
    void on_draw(Graphics::Canvas& canvas) override;

    inline void set_text(const String& text)
    {
        m_text = text;
        request_layout();
    }

    inline void set_text_color(const Graphics::Color& color)
    {
        m_color = color;
        invalidate();
    }
    // TODO: typeface instead of bitmap font
    inline void set_typeface(Graphics::BitmapFont* font)
    {
        m_font = font;
        request_layout();
    }
    inline void set_gravity(Gravity gravity)
    {
        m_gravity = gravity;
        request_layout();
    }

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
