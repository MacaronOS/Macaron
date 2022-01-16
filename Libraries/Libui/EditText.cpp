#include "EditText.hpp"

namespace UI {

bool EditText::on_keyboard_event(const KeyboardEvent& event)
{
    if (event.pressed) {
        if (m_before_text_changed) {
            m_before_text_changed(m_text, m_text.size(), 1, 1);
        }

        m_text.push_back(KeyToAsci(event.key));

        if (m_on_text_changed) {
            m_on_text_changed(m_text, m_text.size() - 1, m_text.size() - 1, 1);
        }

        request_layout();

        if (m_after_text_changed) {
            m_after_text_changed(m_text);
        }
    }

    if (m_on_keyboard) {
        m_on_keyboard(event);
    }

    return true;
}

}