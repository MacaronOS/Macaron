#include "Application.hpp"
#include "Activity.hpp"

namespace UI {

void Application::on_CreateWindowResponse(CreateWindowResponse& response)
{
    m_activity_stack.back()->on_window(response.window_id(), response.shared_buffer_id());
}

void Application::on_MouseMoveRequest(MouseMoveRequest& request)
{
    Event event;
    event.type = EventType::MouseMove;
    event.mouse_move_event = { request.x(), request.y() };

    EventLoop::the().enqueue_callback_for_event([&](const Event& event) {
        m_activity_stack.back()->content_view()->dispatch_mouse_move_event(event.mouse_move_event);
    },
        event);
}

void Application::on_MouseClickRequest(MouseClickRequest& request)
{
    Event event;
    event.type = EventType::MouseClick;
    event.mouse_click_event = { request.x(), request.y() };

    EventLoop::the().enqueue_callback_for_event([&](const Event& event) {
        m_activity_stack.back()->content_view()->dispatch_mouse_click_event(event.mouse_click_event);
    },
        event);
}

void Application::invalidate_area(int x, int y, int width, int height)
{
    m_connection.send_InvalidateRequest(InvalidateRequest(m_activity_stack.back()->window()->id(), 0, 0, width, height));
}

}