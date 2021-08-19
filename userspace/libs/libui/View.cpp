#include "View.hpp"
#include "Application.hpp"
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

void View::invalidate(int l, int t, int r, int b)
{
    // TODO: invalidate should just redraw a particular area inside view
    // and not layout + redraw everly view in a tree
    request_layout();
}

void View::request_layout()
{
    // TODO: currently request_layout calculates layout for every view in a tree
    // this should be done more efficenly by moving from bottom to top
    auto& window = Application::the().window();
    if (!window.content_view()) {
        return;
    }
    auto canvas = Graphics::Canvas(window.buffer());
    window.content_view()->measure(
        View::MeasureSpec::MakeMeasureSpec(window.width(), View::MeasureSpec::EXACTLY),
        View::MeasureSpec::MakeMeasureSpec(window.height(), View::MeasureSpec::EXACTLY));
    window.content_view()->layout(0, 0, m_width - 1, m_height - 1);
    window.content_view()->draw(canvas);
    Application::the().invalidate_area(0, 0, window.width(), window.height());
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

}