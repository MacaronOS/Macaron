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
            remain_width = max(0, remain_width - child->measured_width() - child_params->left_margin - child_params->right_margin);
        } else {
            remain_height = max(0, remain_height - child->measured_height() - child_params->top_margin - child_params->bottom_margin);
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
        auto child_params = static_cast<LinearLayoutParams*>(child->layout_params());

        child->layout(
            child_left + child_params->left_margin,
            child_top + child_params->top_margin,
            child_left + child_params->left_margin + child->measured_width(),
            child_top + child_params->top_margin + child->measured_height());

        if (m_orienttion == Orientation::Horizontal) {
            child_left += (child->width() + child_params->left_margin + child_params->right_margin);
        } else {
            child_top += (child->height() + child_params->top_margin + child_params->bottom_margin);
        }
    }
}

void LinearLayout::on_draw(Graphics::Canvas& canvas)
{
    for (auto child : m_children) {
        canvas.save();

        canvas.clip_rect(child->left(), child->top(), child->right(), child->bottom());
        canvas.move_cursor_by(child->left(), child->top());
        child->draw(canvas);

        canvas.restore();
    }
}

bool LinearLayout::check_layout_params(LayoutParams* params)
{
    return params->type == LayoutParamsType::LinearLayoutParams;
}

LayoutParams* LinearLayout::generate_layout_params(LayoutParams* params)
{   
    if (params->type == LayoutParamsType::LinearLayoutParams) {
        return params;
    }

    auto new_params = new LinearLayoutParams();

    if (params->type == LayoutParamsType::Default) {
        *static_cast<LayoutParams*>(new_params) = *params;

    } else if (params->type == LayoutParamsType::MarginLayoutParams) {
        *static_cast<MarginLayoutParams*>(new_params) = *static_cast<MarginLayoutParams*>(params);
    }

    delete params;
    return new_params;
}

}