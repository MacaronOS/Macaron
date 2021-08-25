#include "ViewGroup.hpp"

namespace UI {

bool ViewGroup::on_mouse_move_event(const MouseMoveEvent& event)
{
    bool handled = false;
    for (auto child : m_children) {
        if (child->bounds().contains(event.x, event.y)) {
            handled |= child->dispatch_mouse_move_event({ event.x - child->left(), event.y - child->top() });
        }
    }
    return handled;
}

bool ViewGroup::on_mouse_click_event(const MouseClickEvent& event)
{
    bool handled = false;
    for (auto child : m_children) {
        if (child->bounds().contains(event.x, event.y)) {
            handled |= child->dispatch_mouse_click_event({ event.x - child->left(), event.y - child->top() });
        }
    }
    return handled;
}

}