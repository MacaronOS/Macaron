#include "WindowServer.hpp"

#include <libc/malloc.hpp>
#include <libc/syscalls.hpp>

#include <libsys/Log.hpp>
#include <libsys/syscalls.hpp>

#include <libgraphics/Bitmap.hpp>
#include <libgraphics/Color.hpp>
#include <libgraphics/ws/Connection.hpp>
#include <libgraphics/ws/wrappers/CreateWindow.hpp>

#include <wisterialib/extras.hpp>
#include <wisterialib/posix/defines.hpp>

bool WindowServer::initialize()
{
    // setup the screen
    int screen_fd = open("/dev/bga", 1, 1);
    if (screen_fd < 0) {
        return false;
    }

    void* screen_buffer = mmap(0, 1024 * 768 * 4 * 2, PROT_NONE, MAP_SHARED, screen_fd, 0);
    if (screen_buffer == (void*)-1) {
        return false;
    }

    auto front_buffer = Graphics::Bitmap((Graphics::Color*)screen_buffer, 1024, 768);
    auto back_buffer = Graphics::Bitmap((Graphics::Color*)screen_buffer + 1024 * 768, 1024, 768);

    m_screen = Screen(screen_fd, move(front_buffer), move(back_buffer));

    return true;
}

// in debug purpose
// make windows spawn in different places
int x_offset = 50;
int y_offset = 50;

void WindowServer::run()
{
    while (true) {
        redraw();
        if (m_connection.has_requests()) {
            auto message = m_connection.recieve_message();

            if (message.type() == WS::WSProtocol::Type::CreateWindowRequest) {
                Log << "Recieved CreateWindowRequst" << endl;

                auto shared_buffer = create_shared_buffer(240 * 180 * 4);
                auto pixel_bitmap = Graphics::Bitmap((Graphics::Color*)shared_buffer.mem, 240, 180);

                auto window = new Window(240, 180, move(pixel_bitmap), shared_buffer.id, x_offset, y_offset);
                x_offset += 20 + 240;
                y_offset += 20 + 180;
                m_windows.push_back(window);

                m_connection.send_response_to(WS::CreateWindowResponse(shared_buffer.id), message.pid_from());
            }
        }
    }
}

void WindowServer::redraw()
{
    draw_background();
    for (auto window : m_windows) {
        for (size_t y = 0; y < window->height() - 50; y++) {
            for (size_t x = 0; x < window->width() - 50; x++) {
                m_screen.back_buffer()[y + window->y()][x + window->x()] = window->buffer()[y][x];
            }
        }
    }
    m_screen.swap_buffers();
}

void WindowServer::draw_background()
{
    for (size_t y = 0; y < 768; y++) {
        for (size_t x = 0; x < 1024; x++) {
            m_screen.back_buffer()[y][x] = Graphics::Color(255, 255, 255);
        }
    }
}