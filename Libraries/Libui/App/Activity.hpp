#pragma once

#include "../View.hpp"
#include "../Window.hpp"
#include "Application.hpp"

namespace UI {

class Activity {
public:
    Activity()
    {
        EventLoop::the().enqueue_callback([this]() {
            Application::the().push_activity_onto_the_stack(this);
            on_create();
            on_start();
        });
    }

    inline Window* window() { return m_window; }
    inline const String& titile() const { return m_titile; }
    inline View* content_view() { return m_content_view; }

    inline void set_content_view(View* view)
    {
        // Determine how much space we need for our content view,
        // then ask to WindowServer to create a window for a content view
        auto params = view->layout_params();
        view->measure(
            View::MeasureSpec::MakeMeasureSpec(params->width, View::MeasureSpec::EXACTLY),
            View::MeasureSpec::MakeMeasureSpec(params->height, View::MeasureSpec::EXACTLY));

        int width = view->measured_width();
        int height = view->measured_height();

        Application::the().create_window(width, height, m_titile);

        m_content_view = view;
        // The next step is on_window function. It's called when WindowServer assigns us a window
    }

    // The second stage of set_content_view_function
    inline void on_window(int window_id, int shared_buffer_id)
    {
        int width = m_content_view->measured_width();
        int height = m_content_view->measured_height();

        m_window = new Window(
            Graphics::Bitmap((Graphics::Color*)get_shared_buffer(shared_buffer_id), width, height),
            window_id, width, height);

        m_content_view->layout(0, 0, width, height);

        auto canvas = Graphics::Canvas(m_window->buffer());
        m_content_view->draw(canvas);

        Application::the().connection().send_InvalidateRequest(
            InvalidateRequest(m_window->id(), 0, 0, width, height));
    }

    inline void set_title(const String& titile)
    {
        m_titile = titile;
        if (Application::the().activity_stack().back() == this) {
            // TODO: inform window server about the new tiitle
        }
    }

protected:
    // ----- entire lifetime -----

    // Every Activiy must implelent on_creaate and
    // call set_content_view function at the of execution
    virtual void on_create() = 0;

    // ----- visible lifetime -----
    virtual void on_start() { }

    // ----- foreground lifetime -----
    virtual void on_resume() { }
    virtual void on_pause() { }
    // ----- foreground lifetime -----

    virtual void on_stop() { }
    // ----- visible lifetime -----

    virtual void on_restrart() { }
    virtual void on_destroy() { }

    // ----- entire lifetime -----

protected:
    String m_titile {};
    View* m_content_view {};
    Window* m_window;
};

}