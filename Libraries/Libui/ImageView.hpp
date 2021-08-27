#pragma once

#include "View.hpp"

#include <Macaronlib/Runtime.hpp>

namespace UI {

class ImageView : public View {
public:
    inline void set_image_bitmap(const Graphics::Bitmap& bitmap) { m_image_bitmap = bitmap; }

    void on_measure(int width_measure_spec, int height_measure_spec) override;
    void on_draw(Graphics::Canvas& canvas) override;

private:
    Graphics::Bitmap m_image_bitmap {};
};

}