#include "Application.hpp"
#include "Events.hpp"

#include <libc/syscalls.hpp>
#include <libsys/Log.hpp>

#include <libsys/syscalls.hpp>

#include <libgraphics/Bitmap.hpp>
#include <libgraphics/Color.hpp>
#include <wisterialib/posix/defines.hpp>

namespace UI {

// in demo purposes
uint8_t rgb[3] {};
uint8_t ptr = 0;

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

    m_window.set_content_view(new View());

    auto canvas = Graphics::Canvas(m_window.buffer());
    if (m_window.m_content_view) {
        m_window.m_content_view->on_draw(canvas);
    }
    m_connection.send_InvalidateRequest(InvalidateRequest(m_window.id(), 0, 0, m_width, m_height));

    // rgb[ptr] = 255;

    // EventLoop::the().register_timer([this]() {
    //     for (int y = 0; y < m_window.height(); y++) {
    //         for (int x = 0; x < m_window.width(); x++) {
    //             m_window.buffer()[y][x] = Graphics::Color(rgb[0], rgb[1], rgb[2]);
    //         }
    //     }

    //     rgb[ptr] = 0;
    //     ptr = (ptr + 1) % 3;
    //     rgb[ptr] = 255;

    //     m_connection.send_InvalidateRequest(InvalidateRequest(m_window.id(), 0, 0, m_width, m_height));
    // },
    //     2000);
}

void Application::on_MouseMoveRequest(MouseMoveRequest& request)
{
    if (!m_window.m_content_view) {
        return;
    }
    
    Event event;
    event.type = EventType::MouseMove;
    event.mouse_move_event = { request.x(), request.y() };

    EventLoop::the().enqueue_callback_for_event([this](const Event& event) {
        m_window.m_content_view->on_mouse_move_event(event.mouse_move_event);
    },
        event);
}

}