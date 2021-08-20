#include "Application.hpp"
#include "Events.hpp"
#include "LinearLayout.hpp"
#include "TextView.hpp"
#include "View.hpp"
#include "Button.hpp"

#include <Libc/Syscalls.hpp>
#include <Libsystem/Log.hpp>

#include <Libsystem/Syscalls.hpp>

#include <Libgraphics/Bitmap.hpp>
#include <Libgraphics/Color.hpp>
#include <Libgraphics/Font/FontLoader.hpp>
#include <Wisterialib/posix/defines.hpp>

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

    m_window.set_content_view(m_content_view);
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