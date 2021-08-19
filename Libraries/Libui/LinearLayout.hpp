#pragma once

#include "ViewGroup.hpp"

namespace UI {

class LinearLayout : public ViewGroup {
public:
    void on_measure(int width_measure_spec, int height_measure_spec) override;
    void on_layout(bool changed, int left, int top, int right, int bottom) override;
    void on_draw(Graphics::Canvas& canvas) override;
};

}