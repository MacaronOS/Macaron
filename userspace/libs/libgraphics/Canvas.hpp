#pragma once

#include "Bitmap.hpp"
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
        state() = { { 0, 0, bitmap.width() - 1, bitmap.height() - 1 }, 0, 0 };
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
        for (int y = state().clip_rect.top; y <= state().clip_rect.bottom; y++) {
            for (int x = state().clip_rect.left; x <= state().clip_rect.right; x++) {
                m_bitmap[y][x] = color;
            }
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