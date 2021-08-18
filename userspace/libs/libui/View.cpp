#include "View.hpp"

#include <libsys/Log.hpp>

namespace UI {

void View::set_padding(int left, int top, int right, int bottom)
{
    m_padding_left = left;
    m_padding_top = top;
    m_padding_right = right;
    m_padding_bottom = bottom;
}

void View::measure(int width_measure_spec, int height_measure_spec)
{
    on_measure(width_measure_spec, height_measure_spec);
}

void View::on_measure(int width_measure_spec, int height_measure_spec)
{
    int width = m_padding_left + m_padding_right;
    int height = m_padding_top + m_padding_bottom;
    set_measured_dimensions(resove_size(width, width_measure_spec), resove_size(height, height_measure_spec));
}

void View::layout(int left, int top, int right, int bottom)
{
    m_left = left;
    m_top = top;
    m_width = right - left;
    m_height = bottom - top;
    on_layout(false, left, top, right, bottom);
}

void View::on_layout(bool changed, int left, int top, int right, int bottom)
{
    m_left = left;
    m_top = top;
    m_width = right - left;
    m_height = bottom - top;
}

void View::draw(Graphics::Canvas& canvas)
{
    canvas.draw_color(m_background_color);
    on_draw(canvas);
}

void View::on_mouse_move_event(const MouseMoveEvent& event)
{
    Log << "view move event " << event.x << " " << event.y << endl;
}

}