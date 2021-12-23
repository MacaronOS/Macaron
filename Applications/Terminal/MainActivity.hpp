
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
        set_title("Terminal");

        constexpr uint32_t width = 460;
        constexpr uint32_t height = 340;

        auto font = new Graphics::BitmapFont(Graphics::FontLoader::load("/ext2/Resources/Roboto12Medium.fnt", "/ext2/Resources/Roboto12Medium.bmp"));

        auto layout = new UI::LinearLayout();
        auto layout_params = new UI::LayoutParams();
        layout_params->width = width;
        layout_params->height = height;
        layout->set_layout_params(layout_params);
        layout->set_orientation(UI::LinearLayout::Orientation::Vertical);

        static uint32_t counter = 0;
        static auto counter_view = new UI::TextView();
        counter_view->set_text("Ameliorated arm64 elf generation by emitting missing specification defined details into the binary. Implemented a set of features that emit additional information required by BOLT binary optimizer as well.");
        counter_view->set_typeface(font);
        counter_view->set_background_color(Graphics::Color(240, 240, 240));
        auto counter_view_paprams = new UI::LayoutParams();
        counter_view_paprams->width = 460;
        counter_view_paprams->height = 320;

        auto increase_button = new UI::Button();
        increase_button->set_text("Click!");
        increase_button->set_typeface(font);
        increase_button->set_gravity(UI::Gravity::Center);
        
        auto increase_button_paprams = new UI::LayoutParams();
        increase_button_paprams->width = 460;
        increase_button_paprams->height = 20;

        layout->add_view(counter_view, counter_view_paprams);
        layout->add_view(increase_button, increase_button_paprams);

        set_content_view(layout);
    }
};