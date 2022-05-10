#include "MainActivity.hpp"

#include <Libc/stdio.h>

void MainActivity::on_create()
{
    set_title("Terminal");
    constexpr uint32_t width = 400;
    constexpr uint32_t height = 300;
    m_font = new Graphics::BitmapFont(Graphics::FontLoader::load("/Resources/Roboto12Medium.fnt", "/Resources/Roboto12Medium.bmp"));

    auto layout = new UI::LinearLayout();
    auto layout_params = new UI::LayoutParams();
    layout_params->width = width;
    layout_params->height = height;
    layout->set_layout_params(layout_params);
    layout->set_orientation(UI::LinearLayout::Orientation::Vertical);

    setup_pty_launch_sh();
    setup_terminal_view(layout);
    setup_input_line(layout);

    set_content_view(layout);
}

void MainActivity::setup_pty_launch_sh()
{
    m_pty_master = open("/dev/ptmx", 1, 1);
    if (!m_pty_master) {
        Log << "could not open a ptmx device" << endl;
        exit(1);
    }

    if (!fork()) {
        char* pty_slave_name = ptsname(m_pty_master);

        close(STDIN);
        close(STDOUT);
        close(STDERR);

        open(pty_slave_name, 1, 1);
        open(pty_slave_name, 1, 1);
        open(pty_slave_name, 1, 1);

        execve("/bin/sh", nullptr, nullptr);
    }
}

void MainActivity::setup_terminal_view(UI::LinearLayout* layout)
{
    m_terminal_view = new UI::TextView();
    m_terminal_view->set_typeface(m_font);
    m_terminal_view->set_background_color(Graphics::Color(240, 240, 240));

    UI::EventLoop::the().register_fd_for_select([&]() {
        char buff[255];
        int sz = read(m_pty_master, buff, sizeof(buff) - 1);
        for (int i = 0; i < sz; i++) {
            m_terminal_buffer.push_back(buff[i]);
        }
        m_terminal_view->set_text(m_terminal_buffer);
    },
        m_pty_master);

    auto terminal_view_params = new UI::LayoutParams();
    terminal_view_params->width = 400;
    terminal_view_params->height = 280;

    layout->add_view(m_terminal_view, terminal_view_params);
}

void MainActivity::setup_input_line(UI::LinearLayout* layout)
{
    static auto input_view = new UI::EditText();
    input_view->set_typeface(m_font);
    input_view->set_text("> ");
    input_view->set_background_color(Graphics::Color(210, 210, 210));

    input_view->set_on_keyboard([&](const UI::KeyboardEvent& kb) {
        if (kb.pressed && kb.key == Key::Enter) {
            m_terminal_buffer += input_view->text();
            m_terminal_view->set_text(m_terminal_buffer);
            write(m_pty_master, input_view->text().c_str() + 2, input_view->text().size() - 2);
            input_view->set_text("> ");
        }
    });

    auto input_view_paprams = new UI::LayoutParams();
    input_view_paprams->width = 400;
    input_view_paprams->height = 20;

    layout->add_view(input_view, input_view_paprams);
}