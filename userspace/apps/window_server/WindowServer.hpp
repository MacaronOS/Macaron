#pragma once

#include "Connection.hpp"
#include "Events.hpp"
#include "Mouse.hpp"
#include "Screen.hpp"
#include "Window.hpp"

#include "Font/FontLoader.hpp"

#include <wisterialib/List.hpp>
#include <wisterialib/ObjectPool.hpp>

#include <libgraphics/Rect.hpp>

class WindowServer : public ServerMessageReciever {
    static constexpr auto windows = 10;

public:
    WindowServer()
        : m_event_loop(EventLoop::the())
    {
    }

    bool initialize();
    void run();

    CreateWindowResponse on_CreateWindowRequest(CreateWindowRequest& request) override;
    void on_InvalidateRequest(InvalidateRequest& request) override;
    void on_CloseWindowResponse(CloseWindowResponse& response) override {};

private:
    void redraw();
    void draw_windows();
    void draw_mouse();
    void draw_background();
    void copy_changes_to_second_buffer();

    void draw_text(const String& text, int x, int y, const Font& font);

    void draw_text(const String& text, Graphics::Bitmap& pixels, int x, int y, const Font& font);

    Window* get_window_by_id(int id);

private:
    Screen m_screen {};
    Graphics::Bitmap m_wallpaper {};
    Graphics::Bitmap m_cursor {};
    Font m_font_medium {};
    Font m_font_bold {};
    Mouse m_mouse {};
    Connection m_connection { Connection("/ext2/ws.socket", *this) };
    List<Window*> m_windows {};
    Vector<Graphics::Rect> m_invalid_areas {};
    EventLoop& m_event_loop;
    bool m_mouse_needs_draw_since_moved { true };
    Window* m_selected_window {};
};