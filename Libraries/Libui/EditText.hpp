#pragma once

#include "TextView.hpp"

namespace UI {

class EditText : public TextView {
public:
    EditText()
        : TextView()
    {
        
    }

    bool on_keyboard_event(const KeyboardEvent& event) override;
};

}
