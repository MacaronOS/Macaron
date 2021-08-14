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
    Log << "run app " << endl;
    m_connection.send_CreateWindowRequest(CreateWindowRequest(240, 180, "application"));
    EventLoop::the().run();
}

void Application::on_CreateWindowResponse(CreateWindowResponse& response)
{
    m_window = Window(
        Graphics::Bitmap((Graphics::Color*)get_shared_buffer(response.shared_buffer_id()), 240, 180),
        response.window_id());

    EventLoop::the().register_timer([this]() {
        for (int y = 0; y < m_window.height(); y++) {
            for (int x = 0; x < m_window.width(); x++) {
                m_window.buffer()[y][x] = Graphics::Color(rgb[0], rgb[1], rgb[2]);
            }
        }

        rgb[ptr] = 0;
        ptr = (ptr + 1) % 3;
        rgb[ptr] = 255;

        m_connection.send_InvalidateRequest(InvalidateRequest(m_window.id(), 0, 0, 240, 180));
    },
        2000);
}

}