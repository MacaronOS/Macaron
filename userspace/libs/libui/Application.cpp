#include "Application.hpp"
#include "Events.hpp"
#include "LinearLayout.hpp"
#include "TextView.hpp"
#include "View.hpp"
#include "Button.hpp"

#include <libc/syscalls.hpp>
#include <libsys/Log.hpp>

#include <libsys/syscalls.hpp>

#include <libgraphics/Bitmap.hpp>
#include <libgraphics/Color.hpp>
#include <libgraphics/Font/FontLoader.hpp>
#include <wisterialib/posix/defines.hpp>

namespace UI {

void Application::run()
{
    m_connection.send_CreateWindowRequest(CreateWindowRequest(m_width, m_height, m_titile));
    EventLoop::the().run();
}

void Application::on_CreateWindowResponse(CreateWindowResponse& response)
{
    m_window = Window(
        Graphics::Bitmap((Graphics::Color*)get_shared_buffer(response.shared_buffer_id()), m_width, m_height),
        response.window_id(), m_width, m_height);

    auto font = new Graphics::BitmapFont(Graphics::FontLoader::load("/ext2/res/Roboto12Medium.fnt", "/ext2/res/Roboto12Medium.bmp"));

    auto layout = new LinearLayout();
    auto layout_params = new LinearLayoutParams();
    layout_params->gravity = LinearLayoutParams::Gravity::Horizontal;
    layout_params->width = m_width;
    layout_params->height = m_height;
    layout->set_layout_params(layout_params);

    auto view1 = new Button();
    view1->set_text("Button Button!");
    view1->set_typeface(font);
    view1->set_on_mouse_click_listener([&](View& view) {
        static_cast<Button&>(view).set_text("button clicked");
    });
    auto layout_params_view_1 = new LayoutParams();
    layout_params_view_1->width = m_width / 2;
    layout_params_view_1->height = 50;

    auto view2 = new View();
    view2->set_background_color(Graphics::Color(0, 255, 125));
    auto layout_params_view_2 = new LayoutParams();
    layout_params_view_2->width = m_width / 2;
    layout_params_view_2->height = m_height;
    view2->set_on_mouse_click_listener([&](View& view) {
        Log << "View2 clicked" << endl;
    });

    layout->add_view(view1, layout_params_view_1);
    layout->add_view(view2, layout_params_view_2);

    m_window.set_content_view(layout);
}

void Application::on_MouseMoveRequest(MouseMoveRequest& request)
{
    if (!m_window.m_content_view) {
        return;
    }

    Event event;
    event.type = EventType::MouseMove;
    event.mouse_move_event = { request.x(), request.y() };

    EventLoop::the().enqueue_callback_for_event([&](const Event& event) {
        m_window.m_content_view->dispatch_mouse_move_event(event.mouse_move_event);
    },
        event);
}

void Application::on_MouseClickRequest(MouseClickRequest& request)
{
    if (!m_window.m_content_view) {
        return;
    }

    Event event;
    event.type = EventType::MouseClick;
    event.mouse_click_event = { request.x(), request.y() };

    EventLoop::the().enqueue_callback_for_event([&](const Event& event) {
        m_window.m_content_view->dispatch_mouse_click_event(event.mouse_click_event);
    },
        event);
}

}