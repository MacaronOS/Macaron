#pragma once

#include "Gravity.hpp"
#include "ViewGroup.hpp"

namespace UI {

class LinearLayout : public ViewGroup {
public:
    enum class Orientation {
        Horizontal,
        Vertical,
    };

    void on_measure(int width_measure_spec, int height_measure_spec) override;
    void on_layout(bool changed, int left, int top, int right, int bottom) override;
    void on_draw(Graphics::Canvas& canvas) override;

    inline void set_orientation(Orientation orientation) { m_orienttion = orientation; }
    inline Orientation orientation() const { return m_orienttion; }

protected:
    bool check_layout_params(LayoutParams* params) override;
    LayoutParams* generate_layout_params(LayoutParams* params) override;

private:
    Orientation m_orienttion;
};

}