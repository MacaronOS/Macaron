#include "EditText.hpp"

namespace UI {

bool EditText::on_keyboard_event(const KeyboardEvent& event)
{
    if (event.pressed) {
        m_text.push_back(KeyToAsci(event.key));
        request_layout();
    }
    return true;
}

}