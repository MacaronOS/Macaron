#pragma once

#include "Bitmap.hpp"
#include "Paint.hpp"
#include "Rect.hpp"

namespace Graphics {

class Canvas {
public:
    Canvas(Bitmap& bitmap)
        : m_bitmap(bitmap)
        , m_clip_rect({ 0, 0, bitmap.width() - 1, bitmap.height() - 1 })
    {
    }

    // Clipping
    inline void clip_rect(const Rect& rect)
    {
        m_clip_rect = m_clip_rect.intersection(rect);
    }
    void clip_rect(int left, int top, int right, int bottom)
    {
        clip_rect(Graphics::Rect(left, top, right, bottom));
    }
    void clip_out_rect(const Rect& rect);

    // Drawing
    void draw_color(const Color& Color)
    {
        for (int y = m_clip_rect.top; y <= m_clip_rect.bottom; y++) {
            for (int x = m_clip_rect.left; x <= m_clip_rect.right; x++) {
                m_bitmap[y][x] = Color;
            }
        }
    }
    void draw_rect(const Rect& rect, const Paint& paint);
    void draw_text();

private:
    Bitmap& m_bitmap;
    Rect m_clip_rect;
};

}