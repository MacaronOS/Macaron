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

    // Drawing
    inline void draw_color(const Color& color)
    {
        for (int y = state().clip_rect.top; y < state().clip_rect.bottom; y++) {
            for (int x = state().clip_rect.left; x < state().clip_rect.right; x++) {
                m_bitmap[y][x].mix_with(color);
            }
        }
    }

    inline void draw_text(const String& text, int x, int y, const Graphics::BitmapFont& font)
    {
        x += state().cursor_x;
        y += state().cursor_y;

        int begin_x = x;

        char last_symbol = 0;
        for (char symbol : text) {
            if (symbol == '\n') {
                y += 15;
                x = begin_x;
                continue;
            }
            auto& descr = font.chars[symbol];

            // dont know why it's shifted by 1
            // may be there's some kind of an error in font generator tool
            for (size_t h = 1; h < descr.height + 1; h++) {
                if (y + descr.yoffset + h > state().clip_rect.bottom) {
                    break;
                }
                for (size_t w = 0; w < descr.width; w++) {
                    if (x + descr.xoffset + font.kerning[last_symbol][symbol] + w > state().clip_rect.right) {
                        y += 15;
                        x = begin_x;
                        break;
                    }
                    m_bitmap[y + descr.yoffset + h][x + descr.xoffset + font.kerning[last_symbol][symbol] + w].mix_with(font.texture[descr.y + h][descr.x + w]);
                }
            }

            x += descr.xadvantage;
            last_symbol = symbol;
        }
    }

    inline void draw_bitmap(const Graphics::Bitmap& bitmap, int xoffset = 0, int yoffset = 0)
    {
        xoffset += state().cursor_x;
        yoffset += state().cursor_y;

        int top = max(state().clip_rect.top, yoffset);
        int left = max(state().clip_rect.left, xoffset);

        for (int y = top; y < state().clip_rect.bottom; y++) {
            if (y - top >= bitmap.height()) {
                break;
            }
            for (int x = left; x < state().clip_rect.right; x++) {
                if (x - left >= bitmap.width()) {
                    break;
                }
                m_bitmap[y][x].mix_with(bitmap[y - yoffset][x - xoffset]);
            }
        }
    }

    inline void draw_rounded_bitmap(const Graphics::Bitmap& bitmap, int radius_top, int radius_bottom, int xoffset = 0, int yoffset = 0)
    {
        int xo = xoffset;
        int yo = yoffset;
        xoffset += state().cursor_x;
        yoffset += state().cursor_y;

        int left = max(state().clip_rect.left, xoffset);
        int top = max(state().clip_rect.top, yoffset);
        int right = state().clip_rect.right;
        int bottom = state().clip_rect.bottom;

        auto draw_rounded_borders = [&](int left, int top, int ox, int oy, int r) {
            int t_bottom = min(bottom, top + r);
            int t_right = min(right, left + r);

            for (int y = top; y < t_bottom; y++) {
                if (y - top >= bitmap.height()) {
                    break;
                }
                for (int x = left; x < t_right; x++) {
                    if (x - left >= bitmap.width()) {
                        break;
                    }
                    if ((x - ox) * (x - ox) + (y - oy) * (y - oy) <= r * r) {
                        m_bitmap[y][x].mix_with(bitmap[y - yoffset][x - xoffset]);
                    }
                }
            }
        };

        if (radius_top && left - xoffset <= radius_top && top - yoffset <= radius_top) {
            draw_rounded_borders(
                left,
                top,
                left + radius_top,
                top + radius_top,
                radius_top);
        }

        if (radius_top && xoffset + bitmap.width() - right <= radius_top && top - yoffset <= radius_top) {
            draw_rounded_borders(
                right - radius_top,
                top,
                right - radius_top - 1,
                top + radius_top - 1,
                radius_top);
        }

        if (radius_bottom && xoffset + bitmap.width() - right <= radius_bottom && yoffset + bitmap.height() - bottom <= radius_bottom) {
            draw_rounded_borders(
                right - radius_bottom,
                bottom - radius_bottom,
                right - radius_bottom - 1,
                bottom - radius_bottom - 1,
                radius_bottom);
        }

        if (radius_bottom && left - xoffset <= radius_bottom && yoffset + bitmap.height() - bottom <= radius_bottom) {
            draw_rounded_borders(
                left,
                bottom - radius_bottom,
                left + radius_bottom,
                bottom - radius_bottom,
                radius_bottom);
        }

        save();
        clip_rect(
            xoffset + radius_top,
            yoffset,
            xoffset + bitmap.width() - radius_top,
            yoffset + radius_top);
        draw_bitmap(bitmap, xo, yo);
        restore();

        save();
        clip_rect(
            xoffset,
            yoffset + radius_top,
            xoffset + bitmap.width(),
            yoffset + bitmap.height() - radius_bottom);
        draw_bitmap(bitmap, xo, yo);
        restore();

        save();
        clip_rect(
            xoffset + radius_bottom,
            yoffset + bitmap.height() - radius_bottom,
            xoffset + bitmap.width() - radius_bottom,
            yoffset + bitmap.height());
        draw_bitmap(bitmap, xo, yo);
        restore();
    }

    void draw_rect(const Rect& rect, const Paint& paint);

private:
    inline State& state() { return m_saved_states[m_saved_state_ptr]; }

private:
    Bitmap& m_bitmap;

    State m_saved_states[64];
    int m_saved_state_ptr {};
};
}