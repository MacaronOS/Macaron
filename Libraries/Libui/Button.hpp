#pragma once

#include "TextView.hpp"
#include <Macaronlib/Function.hpp>

namespace UI {

class Button : public TextView {
public:
    Button()
        : TextView()
    {
        set_padding(5, 5, 5, 5);
        set_background_color(Graphics::Color(215, 215, 215));
        set_gravity(Gravity::Center);
    }
};

}