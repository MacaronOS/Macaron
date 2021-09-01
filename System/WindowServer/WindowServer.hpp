#pragma once

#include "Connection.hpp"
#include "Events.hpp"
#include "Mouse.hpp"
#include "Screen.hpp"
#include "Window.hpp"

#include <Macaronlib/List.hpp>
#include <Macaronlib/ObjectPool.hpp>

#include <Libgraphics/Font/BitmapFont.hpp>
#include <Libgraphics/Rect.hpp>

class WindowServer : public ServerMessageReciever {
    static constexpr auto windows = 10;

public:
    WindowServer()
        : m_event_loop(EventLoop::the())
    {
    }

    bool initialize();
    void run();

    CreateWindowResponse on_CreateWindowRequest(CreateWindowRequest& request, int pid_from) override;
    void on_InvalidateRequest(InvalidateRequest& request, int pid_from) override;
    void on_CloseWindowResponse(CloseWindowResponse& response, int pid_from) override {};
    ScreenSizeResponse on_ScreenSizeRequest(ScreenSizeRequest& request, int pid_from) override;
    void on_SetPositionRequest(SetPositionRequest& request, int pid_from) override;

private:
    void redraw();
    void draw_windows();
    void draw_mouse();
    void draw_background();
    void copy_changes_to_second_buffer();

    Window* get_window_by_id(int id);

private:
    Screen m_screen {};
    Graphics::Bitmap m_wallpaper {};
    Graphics::BitmapFont m_font_medium {};
    Graphics::BitmapFont m_font_bold {};
    Mouse m_mouse {};
    Connection m_connection { Connection("/ext2/ws.socket", *this) };
    List<Window*> m_windows {};
    Vector<Graphics::Rect> m_invalid_areas {};
    EventLoop& m_event_loop;
    bool m_mouse_needs_draw_since_moved { true };
    Window* m_selected_window {};
};