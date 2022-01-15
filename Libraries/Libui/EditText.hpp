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

    // Within "s", the count characters beginning at "start" are about to be replaced by new text with length "after".
    using BeforeTextChanged = Function<void(const String& s, int start, int count, int after)>;

    // Within "s", the "count" characters beginning at "start" have just replaced old text that had length "before".
    using OnTextChanged = Function<void(const String& s, int start, int before, int count)>;

    // Somewhere within s, the text has been changed.
    using AfterTextChanged = Function<void(const String& s)>;

    inline void set_before_text_changed(const BeforeTextChanged& btc) { m_before_text_changed = btc; }
    inline void set_on_text_changed(const OnTextChanged& otc) { m_on_text_changed = otc; }
    inline void set_after_text_changed(const AfterTextChanged& atc) { m_after_text_changed = atc; }

private:
    BeforeTextChanged m_before_text_changed {};
    OnTextChanged m_on_text_changed {};
    AfterTextChanged m_after_text_changed {};
};

}
