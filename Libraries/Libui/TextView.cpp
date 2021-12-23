#include "TextView.hpp"

namespace UI {

void TextView::on_measure(int width_measure_spec, int height_measure_spec)
{
    int width = m_padding_left + m_padding_right;
    int height = m_padding_top + m_padding_bottom;

    int text_width = 0;
    int text_height = 13;

    // TODO: multiple lines
    char last_symbol = 0;
    for (char symbol : m_text) {
        if (symbol == '\n') {
            last_symbol = 0;
            text_height += 15;
            continue;
        }
        auto& descr = m_font->chars[symbol];
        text_width += descr.xadvantage + m_font->kerning[last_symbol][symbol];
        last_symbol = symbol;
    }

    width += text_width;
    height += text_height;

    set_measured_dimensions(resove_size(width, width_measure_spec), resove_size(height, height_measure_spec));
    set_measured_text_dimensions(text_width, text_height);
}

void TextView::on_draw(Graphics::Canvas& canvas)
{
    auto left = m_padding_left;
    auto top = m_padding_top;

    if (m_gravity == Gravity::CenterHorizontal || m_gravity == Gravity::Center) {
        auto delta = max(0, (width() - m_measured_text_width - m_padding_right - m_padding_left) / 2);
        left += delta;
    }

    if (m_gravity == Gravity::CenterVertical || m_gravity == Gravity::Center) {
        auto delta = max(0, (height() - m_measured_text_height - m_padding_bottom - m_padding_top) / 2);
        top += delta;
    }

    canvas.draw_text(m_text, left, top, *m_font);
}

}