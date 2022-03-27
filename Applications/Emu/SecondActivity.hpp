
#pragma once

#include <Libui/App/Activity.hpp>
#include <Libui/Button.hpp>
#include <Libui/LinearLayout.hpp>
#include <Libui/TextView.hpp>

#include <Libgraphics/Bitmap.hpp>
#include <Libgraphics/Color.hpp>
#include <Libgraphics/Font/FontLoader.hpp>

class SecondActivity : public UI::Activity {
protected:
    void on_create() override
    {
        set_title("Emu | SecondActivity");

        constexpr uint32_t width = 2 * 170;
        constexpr uint32_t height = 200;

        auto font = new Graphics::BitmapFont(Graphics::FontLoader::load("/Resources/Roboto12Medium.fnt", "/Resources/Roboto12Medium.bmp"));

        auto layout = new UI::LinearLayout();
        auto layout_params = new UI::LayoutParams();
        layout_params->width = width;
        layout_params->height = height;
        layout->set_layout_params(layout_params);
        layout->set_orientation(UI::LinearLayout::Orientation::Vertical);

        static uint32_t counter = 0;
        static auto counter_view = new UI::TextView();
        counter_view->set_text("Emu | SecondActivity");
        counter_view->set_typeface(font);
        counter_view->set_background_color(Graphics::Color(120, 240, 70));
        counter_view->set_gravity(UI::Gravity::Center);
        auto counter_view_paprams = new UI::LayoutParams();
        counter_view_paprams->width = 2 * 170;
        counter_view_paprams->height = 200;

        layout->add_view(counter_view, counter_view_paprams);

        set_content_view(layout);
    }
};