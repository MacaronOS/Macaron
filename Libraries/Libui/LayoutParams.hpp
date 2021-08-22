#pragma once

namespace UI {

struct LayoutParams {
    enum Size {
        WRAP_CONTENT = -2,
        MATCH_PARENT = -1,
    };
    int width;
    int height;
};

struct MarginLayoutParams : public LayoutParams {
    int left_margin;
    int top_margin;
    int right_margin;
    int bottom_margin;
};

struct LinearLayoutParams : public MarginLayoutParams {
    int weight;
};

}