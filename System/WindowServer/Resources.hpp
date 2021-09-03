#pragma once

#include <Libgraphics/BMP/BMPLoader.hpp>
#include <Libgraphics/Font/FontLoader.hpp>
#include <Libsystem/Log.hpp>

class Resources {
public:
    static inline Resources& the()
    {
        static Resources the {};
        return the;
    }

    // Fonts
    inline Graphics::BitmapFont& font_medium() { return m_font_medium; }
    inline Graphics::BitmapFont& font_bold() { return m_font_bold; }

    // Images
    inline Graphics::Bitmap& mouse_cursor() { return m_mouse_cursor; }
    inline Graphics::Bitmap& wallapaper() { return m_wallpaper; }
    inline Graphics::Bitmap& back_button() { return m_back_button; }
    inline Graphics::Bitmap& maximize_button() { return m_maximize_button; }
    inline Graphics::Bitmap& minimize_button() { return m_minimize_button; }
    inline Graphics::Bitmap& close_button() { return m_close_button; }

private:
    Resources()
    {
        m_font_medium = Graphics::FontLoader::load("/ext2/Resources/Roboto12Medium.fnt", "/ext2/Resources/Roboto12Medium.bmp");
        m_font_bold = Graphics::FontLoader::load("/ext2/Resources/Roboto12Bold.fnt", "/ext2/Resources/Roboto12Bold.bmp");

        m_mouse_cursor = Graphics::BMPLoader::load("/ext2/Resources/cursor.bmp");
        m_wallpaper = Graphics::BMPLoader::load("/ext2/Resources/wallpaper.bmp");
        m_back_button = Graphics::BMPLoader::load("/ext2/Resources/back.bmp");
        m_maximize_button = Graphics::BMPLoader::load("/ext2/Resources/maximize.bmp");
        m_minimize_button = Graphics::BMPLoader::load("/ext2/Resources/minimize.bmp");
        m_close_button = Graphics::BMPLoader::load("/ext2/Resources/close.bmp");
    }

private:
    Graphics::BitmapFont m_font_medium {};
    Graphics::BitmapFont m_font_bold {};

    Graphics::Bitmap m_mouse_cursor {};
    Graphics::Bitmap m_wallpaper {};
    Graphics::Bitmap m_back_button {};
    Graphics::Bitmap m_maximize_button {};
    Graphics::Bitmap m_minimize_button {};
    Graphics::Bitmap m_close_button {};
};