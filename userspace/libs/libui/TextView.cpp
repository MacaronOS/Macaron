#include "TextView.hpp"

namespace UI {

void TextView::on_measure(int width_measure_spec, int height_measure_spec)
{
    int width = m_padding_left + m_padding_right;
    int height = m_padding_top + m_padding_bottom;

    // TODO: multiple lines
    char last_symbol = 0;
    for (int at = 0; at < m_text.size(); at++) {
        char symbol = m_text[at];
        auto& descr = m_font->chars[symbol];
        width += descr.xoffset + descr.xadvantage + descr.width + m_font->kerning[last_symbol][symbol];
        last_symbol = symbol;
    }

    set_measured_dimensions(resove_size(width, width_measure_spec), resove_size(height, height_measure_spec));
}

void TextView::on_draw(Graphics::Canvas& canvas)
{
    canvas.draw_text(m_text, m_padding_left, m_padding_top, *m_font);
}

}