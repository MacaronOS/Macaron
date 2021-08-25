
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
    void add(UI::LinearLayout* layout)
    {
        auto view = new UI::View();
        auto view_params = new UI::MarginLayoutParams();
        view_params->width = 32;
        view_params->height = 32;
        view_params->left_margin = 5;
        view_params->top_margin = 10;
        view_params->right_margin = 5;
        view_params->bottom_margin = 10;
        layout->add_view(view, view_params);
        view->set_background_color(Graphics::Color(255, 255, 255));
    }

    void on_create() override
    {
        set_title("Dock");
        UI::Application::the().make_frameless();

        constexpr uint32_t height = 32 + 10 + 10;

        auto layout = new UI::LinearLayout();

        static int width = 0;
        for (int app = 0 ; app < 6 ; app++) {
            add(layout);
            width += (32 + 2 * 5);
        }

        auto layout_params = new UI::LayoutParams();
        layout_params->width = width;
        layout_params->height = height;
        layout->set_layout_params(layout_params);

        set_content_view(layout);

        UI::Application::the().ask_screen_size([this](int screen_width, int screen_height) {
            UI::Application::the().set_position(this, (screen_width - width) / 2, screen_height - height - 10);
        });
    }
};