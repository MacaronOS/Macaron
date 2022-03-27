#pragma once

#include <Libgraphics/Font/BitmapFont.hpp>
#include <Libgraphics/Font/FontLoader.hpp>

namespace UI::R {

const Graphics::BitmapFont& default_font()
{
    Graphics::BitmapFont s_default_font = Graphics::FontLoader::load(
        "/Resources/Roboto12Medium.fnt",
        "/Resources/Roboto12Medium.bmp");

    return s_default_font;
}

}