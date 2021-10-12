#include "WindowServer.hpp"
#include "Resources.hpp"

#include <Libc/Malloc.hpp>
#include <Libc/Syscalls.hpp>

#include <Libsystem/Log.hpp>
#include <Libsystem/Syscalls.hpp>

#include <Libgraphics/BMP/BMPLoader.hpp>
#include <Libgraphics/Bitmap.hpp>
#include <Libgraphics/Canvas.hpp>
#include <Libgraphics/Color.hpp>

#include <Macaronlib/ABI/Syscalls.hpp>
#include <Macaronlib/Runtime.hpp>

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

    // setup m_wallpaper
    if (m_wallpaper.height() == 0 || m_wallpaper.width() == 0) {
        Log << m_wallpaper.height() << " " << m_wallpaper.width() << endl;
        exit(1);
    }

    auto initialize_buffer = [&]() {
        for (int y = 0; y < 768; y++) {
            for (int x = 0; x < 1024; x++) {
                m_screen.back_buffer()[y][x] = m_wallpaper[y][x];
            }
        }
    };

    initialize_buffer();
    m_screen.swap_buffers();
    initialize_buffer();

    // setup devices
    int mouse_fd = open("/dev/mouse", 1, 1);
    if (mouse_fd < 0) {
        return false;
    }

    m_mouse = Mouse(mouse_fd, 1024 / 2, 768 / 2);
    m_mouse.set_clipping(1024, 768);

    m_event_loop.register_timer([this]() {
        redraw();
    },
        1000 / 60);

    m_event_loop.register_fd_for_select([this]() {
        m_mouse.pump();

        if (m_mouse.x() != m_mouse.prev_x() || m_mouse.y() != m_mouse.prev_y()) {
            m_invalid_areas.push_back(m_mouse.prev_bounds());
            m_mouse_needs_draw_since_moved = true;
        }

        if (m_mouse.pressed()) {
            if (m_selected_window) {
                m_invalid_areas.push_back(m_selected_window->all_bounds().intersection(m_screen.bounds()));

                int del_x = m_mouse.x() - m_mouse.prev_x();
                int del_y = m_mouse.y() - m_mouse.prev_y();

                m_selected_window->move_position(del_x, del_y);

                m_invalid_areas.push_back(m_selected_window->all_bounds().intersection(m_screen.bounds()));
            } else {
                for (auto window : m_windows) {
                    if (window->visibility() == 0) {
                        continue;
                    }
                    if (window->frame_bounds().contains(m_mouse.x(), m_mouse.y())) {
                        m_selected_window = window;
                    }
                }
            }
        } else {
            m_selected_window = nullptr;
        }

        for (auto window : m_windows) {
            if (window->visibility() == 0) {
                continue;
            }
            
            if (window->bounds().contains(m_mouse.x(), m_mouse.y())) {
                m_connection.send_MouseMoveRequest(
                    UI::Protocols::MouseMoveRequest(
                        window->id, m_mouse.x() - window->x(), m_mouse.y() - window->y()),
                    window->pid());
            }
        }

        auto clicks = m_mouse.take_over_clicks();
        for (auto& click : clicks) {
            for (auto window : m_windows) {
                if (window->visibility() == 0) {
                    continue;
                }
                if (window->back_button_clicked(click.x, click.y)) {
                    m_connection.send_BackRequest(UI::Protocols::BackRequest(window->id), window->pid());
                    break;
                }

                if (window->bounds().contains(click.x, click.y)) {
                    m_connection.send_MouseClickRequest(
                        UI::Protocols::MouseClickRequest(
                            window->id, click.x - window->x(), click.y - window->y()),
                        window->pid());
                    break;
                }
            }
        }
    },
        mouse_fd);

    return true;
}

// in debug purpose
// make windows spawn in different places
int x_offset = 50;
int y_offset = 50;

void WindowServer::run()
{
    while (true) {
        m_event_loop.pump();
    }
}

CreateWindowResponse WindowServer::on_CreateWindowRequest(CreateWindowRequest& request, int pid_from)
{
    Log << "Recieved CreateWindowRequst" << endl;
    auto shared_buffer = create_shared_buffer(request.widht() * request.height() * 4);
    auto pixel_bitmap = Graphics::Bitmap((Graphics::Color*)shared_buffer.mem, request.widht(), request.height());

    x_offset += 50;
    y_offset += 50;

    if (y_offset + request.height() >= m_screen.height || x_offset + request.widht() >= m_screen.width) {
        y_offset = 50;
        x_offset = 50;
    }

    auto window = new Window(
        request.titile(), request.widht(), request.height(),
        move(pixel_bitmap), shared_buffer.id, pid_from,
        x_offset, y_offset);
    
    if (request.frameless() > 0) {
        window->make_frameless();
    }

    m_windows.push_back(window);

    m_invalid_areas.push_back(window->frame_bounds());

    return CreateWindowResponse(window->id, shared_buffer.id);
}

void WindowServer::on_MakeWindowVisibleRequest(MakeWindowVisibleRequest& request, int pid_from)
{
    Log << "on_MakeWindowVisibleRequest" << endl;
    
    auto window = get_window_by_id(request.widnow_id());
    window->set_visibility(request.visibility());
    m_invalid_areas.push_back(window->all_bounds().intersection(m_screen.bounds()));
}

void WindowServer::on_DestroyWindowRequest(DestroyWindowRequest& request, int pid_from)
{
    Log << "on_DestroyWindowRequest" << endl;

    auto window = get_window_by_id(request.widnow_id());
    m_invalid_areas.push_back(window->all_bounds().intersection(m_screen.bounds()));
    m_windows.remove(m_windows.find(window));
}

void WindowServer::on_InvalidateRequest(InvalidateRequest& request, int pid_from)
{
    Log << "Recieved InvalidateRequest " << request.window_id() << " " << request.width() << " " << request.height() << endl;

    auto window = get_window_by_id(request.window_id());
    m_invalid_areas.push_back(
        Graphics::Rect(
            window->x() + request.x(), window->y() + request.y(),
            window->x() + request.x() + request.width(), window->y() + request.y() + request.height())
            .intersection(m_screen.bounds()));
}

ScreenSizeResponse WindowServer::on_ScreenSizeRequest(ScreenSizeRequest& request, int pid_from)
{
    return ScreenSizeResponse(m_screen.width, m_screen.height);
}

void WindowServer::on_SetPositionRequest(SetPositionRequest& request, int pid_from)
{
    auto window = get_window_by_id(request.window_id());

    m_invalid_areas.push_back(window->all_bounds().intersection(m_screen.bounds()));
    window->set_position(request.left(), request.top());
    m_invalid_areas.push_back(window->all_bounds().intersection(m_screen.bounds()));
}

void WindowServer::redraw()
{
    draw_background();
    draw_windows();
    draw_mouse();

    m_screen.swap_buffers();
    copy_changes_to_second_buffer();
    m_invalid_areas.clear();
}

void WindowServer::draw_background()
{
    for (size_t at = 0; at < m_invalid_areas.size(); at++) {
        auto invalid_area = m_invalid_areas[at];

        for (size_t y = invalid_area.top; y < invalid_area.bottom; y++) {
            for (size_t x = invalid_area.left; x < invalid_area.right; x++) {
                m_screen.back_buffer()[y][x] = m_wallpaper[y][x];
            }
        }
    }
}

void WindowServer::draw_windows()
{
    for (size_t at = 0; at < m_invalid_areas.size(); at++) {
        auto invalid_area = m_invalid_areas[at];

        for (auto window : m_windows) {
            if (window->visibility() == 0) {
                continue;
            }

            if (invalid_area.intersects(window->bounds())) {
                auto intersection = invalid_area.intersection(window->bounds());

                for (int y = intersection.top; y < intersection.bottom; y++) {
                    for (int x = intersection.left; x < intersection.right; x++) {
                        m_screen.back_buffer()[y][x] = window->buffer()[y - window->y()][x - window->x()];
                    }
                }
            }

            if (invalid_area.intersects(window->frame_bounds())) {
                auto intersection = invalid_area.intersection(window->frame_bounds());

                auto bounds = window->frame_bounds();

                for (int y = intersection.top; y < intersection.bottom; y++) {
                    for (int x = intersection.left; x < intersection.right; x++) {
                        m_screen.back_buffer()[y][x] = window->frame_buffer()[y - bounds.top][x - bounds.left];
                    }
                }
            }
        }
    }
}

void WindowServer::draw_mouse()
{
    if (m_mouse.x() < 0 || m_mouse.y() < 0 || m_mouse.x() >= 1024 || m_mouse.y() >= 768) {
        return;
    }
    auto mouse_rect = m_mouse.bounds().intersection(m_screen.bounds());

    bool intersects = false;
    for (size_t at = 0; at < m_invalid_areas.size(); at++) {
        auto invalid_area = m_invalid_areas[at];
        if (invalid_area.intersects(mouse_rect)) {
            intersects = true;
            break;
        }
    }

    if (intersects || m_mouse_needs_draw_since_moved) {
        for (int y = mouse_rect.top; y < mouse_rect.bottom; y++) {
            for (int x = mouse_rect.left; x < mouse_rect.right; x++) {
                m_screen.back_buffer()[y][x].mix_with(m_mouse.cursor()[y - m_mouse.y()][x - m_mouse.x()]);
            }
        }

        m_mouse_needs_draw_since_moved = false;
    }
}

void WindowServer::copy_changes_to_second_buffer()
{
    for (size_t at = 0; at < m_invalid_areas.size(); at++) {
        auto invalid_area = m_invalid_areas[at];

        for (size_t y = invalid_area.top; y < invalid_area.bottom; y++) {
            for (size_t x = invalid_area.left; x < invalid_area.right; x++) {
                m_screen.back_buffer()[y][x] = m_screen.front_buffer()[y][x];
            }
        }
    }

    auto mouse_rect = m_mouse.bounds().intersection(m_screen.bounds());
    for (int y = mouse_rect.top; y < mouse_rect.bottom; y++) {
        for (int x = mouse_rect.left; x < mouse_rect.right; x++) {
            m_screen.back_buffer()[y][x] = m_screen.front_buffer()[y][x];
        }
    }

    // for (int h = 0; h < m_cursor.height(); h++) {
    //     for (int w = 0; w < m_cursor.width(); w++) {
    //         m_screen.back_buffer()[m_mouse.y() + h][m_mouse.x() + w] = m_screen.front_buffer()[m_mouse.y() + h][m_mouse.x() + w];
    //     }
    // }
}

Window* WindowServer::get_window_by_id(int id)
{
    for (auto* window : m_windows) {
        if (window->id == id) {
            return window;
        }
    }
    return nullptr;
}
