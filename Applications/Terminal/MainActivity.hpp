
#pragma once

#include <Libc/stdlib.h>

#include <Libui/App/Activity.hpp>
#include <Libui/Button.hpp>
#include <Libui/EditText.hpp>
#include <Libui/Events.hpp>
#include <Libui/LinearLayout.hpp>
#include <Libui/TextView.hpp>

#include <Libgraphics/Bitmap.hpp>
#include <Libgraphics/Color.hpp>
#include <Libgraphics/Font/FontLoader.hpp>

class MainActivity : public UI::Activity {
protected:
    void on_create() override;

private:
    void setup_pty_launch_sh();
    void setup_terminal_view(UI::LinearLayout* layout);
    void setup_input_line(UI::LinearLayout* layout);

private:
    Graphics::BitmapFont* m_font {};
    UI::TextView* m_terminal_view {};
    String m_terminal_buffer {};
    int m_pty_master {};
};