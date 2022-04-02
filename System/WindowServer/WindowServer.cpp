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
            memcpy(m_screen.back_buffer()[y], m_wallpaper[y], 1024 * sizeof(Graphics::Color));
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

    int keyoard_fd = open("/dev/kbd", 1, 1);
    if (keyoard_fd < 0) {
        return false;
    }

    m_keyboard = Keyboard(keyoard_fd);

    m_event_loop.register_timer([this]() {
        redraw();
    },
        1000 / 60);

    m_event_loop.register_fd_for_select([this]() {
        m_mouse.pump();

        if (m_mouse.x() != m_mouse.prev_x() || m_mouse.y() != m_mouse.prev_y()) {
            invalidate(m_mouse.prev_bounds());
            m_mouse_needs_draw_since_moved = true;
        }

        if (!m_mouse.pressed()) {
            m_windows.unset_picked_window();
        }

        if (m_mouse.pressed() && !m_windows.picked_window()) {
            m_windows.from_top_to_bottom_visible([&](Windows::Iterator window_it) {
                if (!(*window_it).all_bounds().contains(m_mouse.x(), m_mouse.y())) {
                    return false;
                }

                m_windows.bring_to_top(window_it);
                window_it = m_windows.top();

                invalidate((*window_it).all_bounds().intersection(m_screen.bounds()));

                if ((*window_it).frame_bounds().contains(m_mouse.x(), m_mouse.y())) {
                    m_windows.set_picked_window(window_it);
                }

                return true;
            });
        }

        if (m_windows.picked_window()) {
            auto& picked_window = *m_windows.picked_window();
            invalidate(picked_window.all_bounds().intersection(m_screen.bounds()));

            int del_x = m_mouse.x() - m_mouse.prev_x();
            int del_y = m_mouse.y() - m_mouse.prev_y();
            picked_window.move_position(del_x, del_y);

            invalidate(picked_window.all_bounds().intersection(m_screen.bounds()));
            return;
        }

        auto clicks = m_mouse.take_over_clicks();
        for (auto& click : clicks) {
            m_windows.from_top_to_bottom_visible([&](Windows::Iterator window_it) {
                auto& window = *window_it;

                if (window.bounds().contains(click.x, click.y)) {
                    m_connection.send_MouseClickRequest(
                        UI::Protocols::MouseClickRequest(
                            window.id, click.x - window.x(), click.y - window.y()),
                        window.pid());
                    return true;
                }

                return false;
            });
        }

        if (m_windows.top()) {
            auto& top_window = *m_windows.top();
            if (top_window.bounds().contains(m_mouse.x(), m_mouse.y())) {
                m_connection.send_MouseMoveRequest(
                    UI::Protocols::MouseMoveRequest(
                        top_window.id, m_mouse.x() - top_window.x(), m_mouse.y() - top_window.y()),
                    top_window.pid());
            }
        }
    },
        mouse_fd);

    m_event_loop.register_fd_for_select([this]() {
        m_keyboard.pump();
        if (m_windows.top()) {
            auto& top_window = *m_windows.top();
            auto packets = m_keyboard.take_over_packets();
            for (auto& packet : packets) {
                m_connection.send_KeyRequest(
                    UI::Protocols::KeyRequest((int)packet.key, packet.pressed), top_window.pid());
            }
        }
    },
        keyoard_fd);

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

    auto& window = m_windows.create_window(
        request.titile(), request.widht(), request.height(),
        move(pixel_bitmap), shared_buffer.id, pid_from,
        x_offset, y_offset);

    if (request.frameless() > 0) {
        window.make_frameless();
    }

    invalidate(window.frame_bounds());

    return CreateWindowResponse(window.id, shared_buffer.id);
}

void WindowServer::on_MakeWindowVisibleRequest(MakeWindowVisibleRequest& request, int pid_from)
{
    Log << "on_MakeWindowVisibleRequest" << endl;

    auto window_it = m_windows.find_window_by_id(request.widnow_id());
    if (window_it) {
        auto& window = *window_it;
        window.set_visibility(request.visibility());
        invalidate(window.all_bounds().intersection(m_screen.bounds()));
    }
}

void WindowServer::on_DestroyWindowRequest(DestroyWindowRequest& request, int pid_from)
{
    Log << "on_DestroyWindowRequest" << endl;

    auto window_it = m_windows.find_window_by_id(request.widnow_id());
    if (window_it) {
        auto& window = *window_it;
        invalidate(window.all_bounds().intersection(m_screen.bounds()));
        m_windows.destroy_window(window_it);
    }
}

void WindowServer::on_InvalidateRequest(InvalidateRequest& request, int pid_from)
{
    Log << "Recieved InvalidateRequest " << request.window_id() << " " << request.width() << " " << request.height() << endl;

    auto window_it = m_windows.find_window_by_id(request.window_id());
    if (window_it) {
        auto& window = *window_it;
        invalidate(Graphics::Rect(
            window.x() + request.x(), window.y() + request.y(),
            window.x() + request.x() + request.width(), window.y() + request.y() + request.height())
                       .intersection(m_screen.bounds()));
    }
}

ScreenSizeResponse WindowServer::on_ScreenSizeRequest(ScreenSizeRequest& request, int pid_from)
{
    return ScreenSizeResponse(m_screen.width, m_screen.height);
}

void WindowServer::on_SetPositionRequest(SetPositionRequest& request, int pid_from)
{
    auto window_it = m_windows.find_window_by_id(request.window_id());
    if (window_it) {
        auto& window = *window_it;
        invalidate(window.all_bounds().intersection(m_screen.bounds()));
        window.set_position(request.left(), request.top());
        invalidate(window.all_bounds().intersection(m_screen.bounds()));
    }
}

void WindowServer::redraw()
{
    if (!m_invalid_areas.size()) {
        return;
    }

    draw_background();
    draw_windows();
    draw_mouse();

    m_screen.swap_buffers();
    copy_changes_to_second_buffer();
    m_invalid_areas.clear();
}

void WindowServer::draw_background()
{
    for (auto& invalid_area : m_invalid_areas) {
        for (size_t y = invalid_area.top; y < invalid_area.bottom; y++) {
            for (size_t x = invalid_area.left; x < invalid_area.right; x++) {
                m_screen.back_buffer()[y][x] = m_wallpaper[y][x];
            }
        }
    }
}

void WindowServer::draw_windows()
{
    for (auto& window : m_windows) {
        for (auto& invalid_area : m_invalid_areas) {
            if (invalid_area.intersects(window.bounds())) {
                auto intersection = invalid_area.intersection(window.bounds());

                for (int y = intersection.top; y < intersection.bottom; y++) {
                    memcpy(&m_screen.back_buffer()[y][intersection.left],
                        &window.buffer()[y - window.y()][intersection.left - window.x()],
                        intersection.width() * sizeof(Graphics::Color));
                }
            }

            auto frame_bounds = window.frame_bounds();

            if (invalid_area.intersects(frame_bounds)) {
                auto intersection = invalid_area.intersection(frame_bounds);

                for (int y = intersection.top; y < intersection.bottom; y++) {
                    memcpy(&m_screen.back_buffer()[y][intersection.left],
                        &window.frame_buffer()[y - frame_bounds.top][intersection.left - frame_bounds.left],
                        intersection.width() * sizeof(Graphics::Color));
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
    for (auto& invalid_area : m_invalid_areas) {
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
    for (auto& invalid_area : m_invalid_areas) {
        for (size_t y = invalid_area.top; y < invalid_area.bottom; y++) {
            memcpy(&m_screen.back_buffer()[y][invalid_area.left],
                &m_screen.front_buffer()[y][invalid_area.left],
                invalid_area.width() * sizeof(Graphics::Color));
        }
    }

    auto mouse_rect = m_mouse.bounds().intersection(m_screen.bounds());
    for (int y = mouse_rect.top; y < mouse_rect.bottom; y++) {
        memcpy(&m_screen.back_buffer()[y][mouse_rect.left],
            &m_screen.front_buffer()[y][mouse_rect.left],
            mouse_rect.width() * sizeof(Graphics::Color));
    }
}

void WindowServer::invalidate(const Graphics::Rect& rect)
{
    for (auto& other : m_invalid_areas) {
        if (other.contains(rect)) {
            return;
        }
    }

    for (auto& other : m_invalid_areas) {
        auto uni = other.union_rect(rect);
        if (uni.area() < other.area() + rect.area()) {
            other = uni;
            return;
        }
    }

    m_invalid_areas.push_back(rect);
}
