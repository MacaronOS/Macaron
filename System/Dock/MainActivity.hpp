
#pragma once

#include <Libui/App/Activity.hpp>
#include <Libui/App/Application.hpp>
#include <Libui/Button.hpp>
#include <Libui/LinearLayout.hpp>
#include <Libui/TextView.hpp>

#include <Libgraphics/Bitmap.hpp>
#include <Libgraphics/Color.hpp>
#include <Libgraphics/Font/FontLoader.hpp>

class MainActivity : public UI::Activity {
protected:
    void on_create() override
    {
        set_title("Dock");
        UI::Application::the().make_frameless();

        constexpr uint32_t width = 1024 / 2;
        constexpr uint32_t height = 32 + 10 + 10;

        auto view = new UI::View();
        auto view_params = new UI::LayoutParams();
        view_params->width = width;
        view_params->height = height;
        view->set_layout_params(view_params);
        view->set_background_color(Graphics::Color(255, 255, 255));

        set_content_view(view);

        UI::Application::the().ask_screen_size([this](int screen_width, int screen_height) {
            UI::Application::the().set_position(this, (screen_width - width) / 2, screen_height - height - 10);
        });
    }
};