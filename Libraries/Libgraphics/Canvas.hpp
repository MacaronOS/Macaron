#pragma once

#include "Bitmap.hpp"
#include "Font/BitmapFont.hpp"
#include "Paint.hpp"
#include "Rect.hpp"

namespace Graphics {

class Canvas {
    struct State {
        Rect clip_rect; // cliped rectangle in global coordinates
        int cursor_x;
        int cursor_y;
    };

public:
    Canvas(Bitmap& bitmap)
        : m_bitmap(bitmap)
    {
        state() = { { 0, 0, bitmap.width(), bitmap.height() }, 0, 0 };
    }

    inline int save()
    {
        m_saved_states[m_saved_state_ptr + 1] = state();
        return m_saved_state_ptr++;
    }

    inline void restore()
    {
        m_saved_state_ptr--;
    }

    inline void move_cursor_by(int offset_x, int offset_y)
    {
        state().cursor_x += offset_x;
        state().cursor_y += offset_y;
    }

    // Clipping
    inline void clip_rect(const Rect& rect)
    {
        auto rect_in_global_coordinates = rect;
        rect_in_global_coordinates.left += state().cursor_x;
        rect_in_global_coordinates.top += state().cursor_y;
        rect_in_global_coordinates.right += state().cursor_x;
        rect_in_global_coordinates.bottom += state().cursor_y;

        state().clip_rect = state().clip_rect.intersection(rect_in_global_coordinates);
    }
    inline void clip_rect(int left, int top, int right, int bottom)
    {
        clip_rect(Graphics::Rect(left, top, right, bottom));
    }
    void clip_out_rect(const Rect& rect);

    // Drawing
    inline void draw_color(const Color& color)
    {
        for (int y = state().clip_rect.top; y < state().clip_rect.bottom; y++) {
            for (int x = state().clip_rect.left; x < state().clip_rect.right; x++) {
                m_bitmap[y][x] = color;
            }
        }
    }

    inline void draw_text(const String& text, int x, int y, const Graphics::BitmapFont& font)
    {
        x += state().cursor_x;
        y += state().cursor_y;

        char last_symbol = 0;
        for (size_t i = 0; i < text.size(); i++) {
            char symbol = text[i];
            auto& descr = font.chars[symbol];

            // dont know why it's shifted by 1
            // may be there's some kind of an error in font generator tool
            for (size_t h = 1; h < descr.height + 1; h++) {
                if (y + descr.yoffset + h > state().clip_rect.bottom) {
                    break;
                }
                for (size_t w = 0; w < descr.width; w++) {
                    if (x + descr.xoffset + font.kerning[last_symbol][symbol] + w > state().clip_rect.right) {
                        break;
                    }
                    m_bitmap[y + descr.yoffset + h][x + descr.xoffset + font.kerning[last_symbol][symbol] + w].mix_with(font.texture[descr.y + h][descr.x + w]);
                }
            }

            x += descr.xadvantage;
            last_symbol = symbol;
        }
    }

    void draw_rect(const Rect& rect, const Paint& paint);
    void draw_text();

private:
    inline State& state() { return m_saved_states[m_saved_state_ptr]; }

private:
    Bitmap& m_bitmap;

    State m_saved_states[64];
    int m_saved_state_ptr {};
};
}