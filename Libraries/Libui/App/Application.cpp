#include "Application.hpp"
#include "Activity.hpp"

namespace UI {

void Application::on_CreateWindowResponse(CreateWindowResponse& response)
{
    auto activity_it = m_pending_on_window.rbegin();
    (*activity_it)->on_window(response.window_id(), response.shared_buffer_id());
    m_pending_on_window.remove(activity_it);
}

void Application::on_ScreenSizeResponse(ScreenSizeResponse& response)
{
    for (auto& callback : m_on_screen_size_callbacks) {
        callback(response.width(), response.height());
    }
    m_on_screen_size_callbacks.clear();
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

void Application::create_window(int width, int height, const String& titile)
{
    m_connection.send_CreateWindowRequest(CreateWindowRequest(width, height, m_frameless, titile));
}

void Application::enqueue_on_window(int width, int height, const String& titile, Activity* activity)
{
    m_pending_on_window.push_back(activity);
    m_connection.send_CreateWindowRequest(CreateWindowRequest(width, height, m_frameless, titile));
}

void Application::ask_screen_size(const Function<void(int width, int height)>& callback)
{
    m_on_screen_size_callbacks.push_back(callback);
    if (m_on_screen_size_callbacks.size() == 1) {
        m_connection.send_ScreenSizeRequest(ScreenSizeRequest());
    }
}

void Application::set_position(Activity* activity, int left, int top)
{
    m_connection.send_SetPositionRequest(SetPositionRequest(activity->window()->id(), left, top));
}

void Application::on_BackRequest(BackRequest& request)
{
    auto activivty = m_activity_stack.back();
    activivty->on_stop();
    activivty->on_destroy();

    m_connection.send_DestroyWindowRequest(
        UI::Protocols::DestroyWindowRequest(activivty->window()->id()));

    m_activity_stack.pop_back();

    if (m_activity_stack.size() == 0) {
        exit(0);
    }

    auto resumed_activity = m_activity_stack.back();
    resumed_activity->on_resume();
    m_connection.send_MakeWindowVisibleRequest(
        UI::Protocols::MakeWindowVisibleRequest(resumed_activity->window()->id(), 255));
}

void Application::register_new_activity(Activity* activity)
{
    if (m_activity_stack.size() > 0) {
        auto stopped_activity = m_activity_stack.back();
        stopped_activity->on_pause();
        m_connection.send_MakeWindowVisibleRequest(
            UI::Protocols::MakeWindowVisibleRequest(stopped_activity->window()->id(), 0));
    }
    m_activity_stack.push_back(activity);
}

}