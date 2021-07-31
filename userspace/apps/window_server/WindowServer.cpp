#include "WindowServer.hpp"
#include "BMP/BMPLoader.hpp"
#include "Font/FontLoader.hpp"

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
    m_font_medium = FontLoader::load("/ext2/res/Roboto12Medium.fnt", "/ext2/res/Roboto12Medium.bmp");
    m_font_bold = FontLoader::load("/ext2/res/Roboto12Bold.fnt", "/ext2/res/Roboto12Bold.bmp");
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

    // setup wallpaper
    m_wallpaper = BMPLoader::load("/ext2/res/wallpaper.bmp");
    if (m_wallpaper.height() == 0 || m_wallpaper.width() == 0) {
        Log << m_wallpaper.height() << " " << m_wallpaper.width() << endl;
        exit(1);
    }

    auto initialize_buffer = [this]() {
        for (int y = 0; y < 768; y++) {
            for (int x = 0; x < 1024; x++) {
                m_screen.back_buffer()[y][x] = m_wallpaper[y][x];
            }
        }
        draw_text(String("Drawing test string using ") + m_font_medium.name + " medium font 12345 ...", 300, 600, m_font_medium);
        draw_text(String("Drawing test string using ") + m_font_bold.name + " bold font 12345 ...", 300, 620, m_font_bold);
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

    m_cursor = BMPLoader::load("/ext2/res/cursor.bmp");
    if (m_cursor.height() == 0 || m_cursor.width() == 0) {
        Log << m_cursor.height() << " " << m_cursor.width() << endl;
        exit(1);
    }

    m_event_loop.register_timer([](){
        Log << "Timer" << endl;
    }, 1000);

    m_event_loop.register_fd_for_select([](){
        Log << "mouse ready" << endl; 
    }, mouse_fd);

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
        m_mouse.update_position();

        if (m_mouse.x() != m_mouse.prev_x() || m_mouse.y() != m_mouse.prev_y()) {
            m_invalid_areas.push_back(Graphics::Rect(m_mouse.prev_x(), m_mouse.prev_y(), m_mouse.prev_x() + m_cursor.width() - 1, m_mouse.prev_y() + m_cursor.height() - 1));
        }

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

                m_connection.send_response_to(WS::CreateWindowResponse(shared_buffer.id, window->id), message.pid_from());

            } else if (message.type() == WS::WSProtocol::Type::InvalidateWindowRequst) {
                auto window_id = message.m_args[0];
                auto x = message.m_args[1];
                auto y = message.m_args[2];
                auto width = message.m_args[3];
                auto height = message.m_args[4];

                // Log << "Recieved InvalidateWindowRequst " << window_id << " " << x << " " << y << " " << width << " " << height << endl;

                auto window = get_window_by_id(window_id);

                m_invalid_areas.push_back(Graphics::Rect(window->x() + x, window->y() + y, window->x() + x + width - 1, window->y() + y + height - 1));

            } else {
                Log << "Unknown message " << (uint32_t)message.type() << " " << message.m_pid_from << " " << message.m_pid_to << endl;
            }
        }
    }
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

        for (size_t y = invalid_area.top; y <= invalid_area.bottom; y++) {
            for (size_t x = invalid_area.left; x <= invalid_area.right; x++) {
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
            if (invalid_area.intersects(window->bounds())) {
                auto intersection = invalid_area.intersection(window->bounds());

                for (int y = intersection.top; y <= intersection.bottom; y++) {
                    for (int x = intersection.left; x <= intersection.right; x++) {
                        m_screen.back_buffer()[y][x] = window->buffer()[y - window->y()][x - window->x()];
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
    auto mouse_rect = Graphics::Rect(m_mouse.x(), m_mouse.y(), m_mouse.x() + m_cursor.width(), m_mouse.y() + m_cursor.height());
    bool intersects = false;
    for (size_t at = 0; at < m_invalid_areas.size(); at++) {
        auto invalid_area = m_invalid_areas[at];
        if (invalid_area.intersects(mouse_rect)) {
            intersects = true;
            break;
        }
    }

    if (intersects || m_mouse.x() != m_mouse.prev_x() || m_mouse.y() != m_mouse.prev_y()) {
        for (int h = 0; h < m_cursor.height(); h++) {
            for (int w = 0; w < m_cursor.width(); w++) {
                m_screen.back_buffer()[m_mouse.y() + h][m_mouse.x() + w].mix_with(m_cursor[h][w]);
            }
        }
    }
}

void WindowServer::copy_changes_to_second_buffer()
{
    for (size_t at = 0; at < m_invalid_areas.size(); at++) {
        auto invalid_area = m_invalid_areas[at];

        for (size_t y = invalid_area.top; y <= invalid_area.bottom; y++) {
            for (size_t x = invalid_area.left; x <= invalid_area.right; x++) {
                m_screen.back_buffer()[y][x] = m_screen.front_buffer()[y][x];
            }
        }
    }

    for (int h = 0; h < m_cursor.height(); h++) {
        for (int w = 0; w < m_cursor.width(); w++) {
            m_screen.back_buffer()[m_mouse.y() + h][m_mouse.x() + w] = m_screen.front_buffer()[m_mouse.y() + h][m_mouse.x() + w];
        }
    }
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

void WindowServer::draw_text(const String& text, int pos_x, int pos_y, const Font& font)
{
    char last_symbol = 0;
    for (size_t i = 0; i < text.size(); i++) {
        char symbol = text[i];
        auto& descr = font.chars[symbol];

        // dont know why it's shifted by 1
        // may be there's some kind of an error in font generator tool
        for (size_t h = 1; h < descr.height + 1; h++) {
            for (size_t w = 0; w < descr.width; w++) {
                m_screen.back_buffer()[pos_y + descr.yoffset + h][pos_x + descr.xoffset + font.kerning[last_symbol][symbol] + w].mix_with(font.texture[descr.y + h][descr.x + w]);
            }
        }

        pos_x += descr.xadvantage;
        last_symbol = symbol;
    }
}