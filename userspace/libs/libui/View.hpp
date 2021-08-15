#pragma once

#include "Events.hpp"
#include <libgraphics/Canvas.hpp>

namespace UI {
class View {
public:
    // Drawing
    virtual void on_draw(Graphics::Canvas& canvas);

    // Event processing
    virtual void on_mouse_move_event(const MouseMoveEvent& event);
    virtual void on_mouse_click_event() {};
    virtual void on_keydown() {}
    virtual void on_keyup() {}
};
}