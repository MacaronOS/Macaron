#pragma once

namespace UI {

enum class LayoutParamsType {
    Default,
    MarginLayoutParams,
    LinearLayoutParams,
};

struct LayoutParams {
    LayoutParams() = default;
    LayoutParams(LayoutParamsType type)
        : type(type)
    {
    }
    enum Size {
        WRAP_CONTENT = -2,
        MATCH_PARENT = -1,
    };
    int width {};
    int height {};
    LayoutParamsType type {};
};

struct MarginLayoutParams : public LayoutParams {
    using LayoutParams::LayoutParams;

    MarginLayoutParams()
        : LayoutParams(LayoutParamsType::MarginLayoutParams)
    {
    }
    int left_margin {};
    int top_margin {};
    int right_margin {};
    int bottom_margin {};
};

struct LinearLayoutParams : public MarginLayoutParams {
    using MarginLayoutParams::MarginLayoutParams;

    LinearLayoutParams()
        : MarginLayoutParams(LayoutParamsType::LinearLayoutParams)
    {
    }
    int weight {};
};

}