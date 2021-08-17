#pragma once

#include "Events.hpp"
#include "LayoutParams.hpp"
#include <libgraphics/Canvas.hpp>
#include <libgraphics/Color.hpp>

namespace UI {

class View {
public:
    class MeasureSpec {
    public:
        enum Spec {
            UNSPECIFIED,
            EXACTLY,
            AT_MOST,
        };
        static inline int GetMode(int measure_spec)
        {
            return measure_spec & 0b11;
        }
        static inline int GetSize(int measure_spec)
        {
            return measure_spec >> 2;
        }

        static inline int MakeMeasureSpec(int size, int mode)
        {
            return (size << 2) | mode;
        }
    };

public:
    void set_padding(int left, int top, int right, int bottom);
    inline void set_background_color(const Graphics::Color& color) { m_background_color = color; }

    // Events
    virtual void on_mouse_move_event(const MouseMoveEvent& event);
    virtual void on_mouse_click_event() {};
    virtual void on_keydown() { }
    virtual void on_keyup() { }
    virtual void on_attached_to_vindow() { }

    // Rendering
    void measure(int width_measure_spec, int height_measure_spec);
    virtual void on_measure(int width_measure_spec, int height_measure_spec);

    void layout(int left, int top, int right, int bottom);
    virtual void on_layout(bool changed, int left, int top, int right, int bottom);

    void draw(Graphics::Canvas& canvas);
    virtual void on_draw(Graphics::Canvas& canvas) { }

    // Layout
    inline void set_layout_params(LayoutParams* layout_params) { m_layout_params = layout_params; }
    inline LayoutParams* layout_params() { return m_layout_params; }

    // Position of the view relative to its parent.
    inline int left() const { return m_left; }
    inline int top() const { return m_top; }
    // The right and bottom edges
    inline int right() const { return m_left + m_width; }
    inline int bottom() const { return m_top + m_height; }

    // Define how big a view wants to be within its parent
    inline int measured_width() const { return m_measured_width; }
    inline int measured_height() const { return m_measured_height; }
    // Define the actual size of the view on screen, at drawing time and after layout
    inline int width() const { return m_width; }
    inline int height() const { return m_height; }

protected:
    inline void set_measured_dimensions(int width, int height)
    {
        m_measured_width = width;
        m_measured_height = height;
    }

    inline int resove_size(int size, int measure_spec)
    {
        int measure_spec_size = MeasureSpec::GetSize(measure_spec);
        int measure_spec_mode = MeasureSpec::GetMode(measure_spec);

        if (measure_spec_mode == MeasureSpec::EXACTLY) {
            return measure_spec_size;
        }
        if (measure_spec_mode == MeasureSpec::AT_MOST) {
            return min(size, measure_spec_size);
        }
        return size;
    }

protected:
    LayoutParams* m_layout_params {};
    View* m_children[20];
    int m_left {};
    int m_top {};
    int m_measured_width {};
    int m_measured_height {};
    int m_width {};
    int m_height {};
    int m_padding_left {};
    int m_padding_top {};
    int m_padding_right {};
    int m_padding_bottom {};
    Graphics::Color m_background_color {};
};
}