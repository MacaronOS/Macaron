#include "Application.hpp"
#include "Bitmap.hpp"
#include "Color.hpp"

#include "ws/Connection.hpp"
#include "ws/wrappers/CreateWindow.hpp"

#include <libc/syscalls.hpp>

#include <libsys/Log.hpp>
#include <libsys/syscalls.hpp>

#include <wisterialib/posix/defines.hpp>

#include "ws/WSProtocol.hpp"

Application::Application()
{
    auto response = m_connection.send_sync_request<WS::CreateWindowRequest, WS::CreateWindowResponse>(WS::CreateWindowRequest());
    m_window = Graphics::Window(
        Graphics::Bitmap((Graphics::Color*)get_shared_buffer(response.buffer_id()), 240, 180),
        response.window_id()
    );
}

void Application::invalidate_area(int x, int y, int width, int height) {
    auto protocol = WS::WSProtocol(WS::WSProtocol::Type::InvalidateWindowRequst, m_window.id(), x, y, width, height);
    m_connection.send_async_message(protocol);
}