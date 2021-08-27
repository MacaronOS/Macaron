#include "ImageView.hpp"

namespace UI {

void ImageView::on_measure(int width_measure_spec, int height_measure_spec)
{
    set_measured_dimensions(
        resove_size(m_image_bitmap.width(), width_measure_spec),
        resove_size(m_image_bitmap.height(), height_measure_spec));
}

void ImageView::on_draw(Graphics::Canvas& canvas)
{
    canvas.draw_bitmap(m_image_bitmap);
}

}