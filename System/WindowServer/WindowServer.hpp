#pragma once

#include "Connection.hpp"
#include "Events.hpp"
#include "Keyboard.hpp"
#include "Mouse.hpp"
#include "Screen.hpp"
#include "Window.hpp"
#include "Windows.hpp"

#include <Macaronlib/List.hpp>
#include <Macaronlib/ObjectPool.hpp>

#include <Libgraphics/Canvas.hpp>
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
    void on_MakeWindowVisibleRequest(MakeWindowVisibleRequest& request, int pid_from) override;
    void on_DestroyWindowRequest(DestroyWindowRequest& request, int pid_from) override;
    void on_InvalidateRequest(InvalidateRequest& request, int pid_from) override;
    void on_CloseWindowResponse(CloseWindowResponse& response, int pid_from) override {};
    ScreenSizeResponse on_ScreenSizeRequest(ScreenSizeRequest& request, int pid_from) override;
    void on_SetPositionRequest(SetPositionRequest& request, int pid_from) override;

private:
    void redraw();
    void draw_windows(Graphics::Canvas& canvas);
    void draw_mouse();
    void draw_background();
    void copy_changes_to_second_buffer();

    void invalidate(const Graphics::Rect& rect);

private:
    Screen m_screen {};
    Graphics::Bitmap& m_wallpaper { Resources::the().wallapaper() };
    Mouse m_mouse {};
    Keyboard m_keyboard {};
    Connection m_connection { Connection("/ws.socket", *this) };
    Windows m_windows {};
    Vector<Graphics::Rect> m_invalid_areas {};
    EventLoop& m_event_loop;
    bool m_mouse_needs_draw_since_moved { true };
};