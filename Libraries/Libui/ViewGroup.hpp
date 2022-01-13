#pragma once

#include "View.hpp"

#include <Macaronlib/Vector.hpp>

namespace UI {

class ViewGroup : public View {
public:
    inline void add_view(View* view, LayoutParams* layout_params)
    {
        if (!check_layout_params(layout_params)) {
            layout_params = generate_layout_params(layout_params);
        }
        view->set_layout_params(layout_params);
        m_children.push_back(view);
    }

    bool on_mouse_move_event(const MouseMoveEvent& event) override;
    bool on_mouse_click_event(const MouseClickEvent& event) override;
    bool on_keyboard_event(const KeyboardEvent& event) override;

protected:
    // Validates layout params of its children
    virtual bool check_layout_params(LayoutParams* params) { return true; };
    // Returns a safe set of layout parameters based on the supplied layout params
    virtual LayoutParams* generate_layout_params(LayoutParams* params) { return nullptr; }
    // These parameters are requested when the View has no layout already set
    virtual LayoutParams* generate_default_layout_params() { return nullptr; }

protected:
    View* m_active_view {};
    Vector<View*> m_children {};
    LayoutParams* layout_params {};
};

}
