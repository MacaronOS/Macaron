#include "LinearLayout.hpp"

namespace UI {

void LinearLayout::on_measure(int width_measure_spec, int height_measure_spec)
{
    auto my_params = static_cast<LayoutParams*>(m_layout_params);

    int remain_width = my_params->width;
    int remain_height = my_params->height;

    for (auto child : m_children) {
        auto child_params = static_cast<LinearLayoutParams*>(child->layout_params());

        if (child_params->height == LinearLayoutParams::MATCH_PARENT || child_params->height == LinearLayoutParams::WRAP_CONTENT) {
            // TODO: NOT SUPPORTED FOR NOW
            continue;
        }

        child->measure(
            MeasureSpec::MakeMeasureSpec(child_params->width, MeasureSpec::EXACTLY),
            MeasureSpec::MakeMeasureSpec(child_params->height, MeasureSpec::EXACTLY));

        if (m_orienttion == Orientation::Horizontal) {
            remain_width = max(0, remain_width - child->measured_width());
        } else {
            remain_height = max(0, remain_height - child->measured_height());
        }
    }

    set_measured_dimensions(
        resove_size(my_params->width - remain_width, width_measure_spec),
        resove_size(my_params->height - remain_height, height_measure_spec));
}

void LinearLayout::on_layout(bool changed, int left, int top, int right, int bottom)
{
    int child_left = 0;
    int child_top = 0;

    auto my_params = static_cast<LinearLayoutParams*>(m_layout_params);

    for (auto child : m_children) {
        child->layout(child_left, child_top, child_left + child->measured_width(), child_top + child->measured_height());

        if (m_orienttion == Orientation::Horizontal) {
            child_left += child->width();
        } else {
            child_top += child->height();
        }
    }
}

void LinearLayout::on_draw(Graphics::Canvas& canvas)
{
    for (auto child : m_children) {
        canvas.save();

        canvas.clip_rect(child->left(), child->top(), child->right() - 1, child->bottom() - 1);
        canvas.move_cursor_by(child->left(), child->top());
        child->draw(canvas);

        canvas.restore();
    }
}

}