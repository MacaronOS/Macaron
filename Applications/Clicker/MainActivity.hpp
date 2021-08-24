
#pragma once

#include <Libui/App/Activity.hpp>
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
        set_title("Clicker");

        constexpr uint32_t width = 170;
        constexpr uint32_t height = 200;

        auto font = new Graphics::BitmapFont(Graphics::FontLoader::load("/ext2/Resources/Roboto12Medium.fnt", "/ext2/Resources/Roboto12Medium.bmp"));

        auto layout = new UI::LinearLayout();
        auto layout_params = new UI::LayoutParams();
        layout_params->width = width;
        layout_params->height = height;
        layout->set_layout_params(layout_params);
        layout->set_orientation(UI::LinearLayout::Orientation::Vertical);

        static uint32_t counter = 0;
        static auto counter_view = new UI::TextView();
        counter_view->set_text("0");
        counter_view->set_typeface(font);
        counter_view->set_background_color(Graphics::Color(240, 240, 240));
        counter_view->set_gravity(UI::Gravity::Center);
        auto counter_view_paprams = new UI::LayoutParams();
        counter_view_paprams->width = 170;
        counter_view_paprams->height = 100;

        auto increase_button = new UI::Button();
        increase_button->set_text("Click!");
        increase_button->set_typeface(font);
        increase_button->set_gravity(UI::Gravity::Center);
        increase_button->set_on_mouse_click_listener([&](UI::View& view) {
            counter_view->set_text(String::From(++counter));
        });
        auto increase_button_paprams = new UI::LayoutParams();
        increase_button_paprams->width = 170;
        increase_button_paprams->height = 100;

        layout->add_view(counter_view, counter_view_paprams);
        layout->add_view(increase_button, increase_button_paprams);

        set_content_view(layout);
    }
};