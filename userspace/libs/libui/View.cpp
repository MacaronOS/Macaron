#include "View.hpp"

#include <libsys/Log.hpp>

namespace UI {

void View::on_draw(Graphics::Canvas& canvas)
{
    canvas.draw_color(Graphics::Color(255, 200, 100));
}

void View::on_mouse_move_event(const MouseMoveEvent& event)
{
    Log << "view move event " << event.x << " " << event.y << endl;
    // Log << "done "
}

}