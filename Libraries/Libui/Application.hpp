#pragma once

#include "Connection.hpp"
#include "Window.hpp"

namespace UI {

class Application : public ClientMessageReciever {
public:
    static Application& the()
    {
        static Application the;
        return the;
    }

    // intiial window properties
    void set_intitial_window_width(int width) { m_width = width; }
    void set_intitial_window_height(int heigth) { m_height = heigth; }
    void set_intitial_window_titile(String titile) { m_titile = move(titile); }

    inline void set_contnt_view(View* content_view) { m_content_view = content_view; }

    void run();

    // protocols
    void on_MouseClickRequest(MouseClickRequest& request) override;
    void on_MouseMoveRequest(MouseMoveRequest& request) override;
    CloseWindowResponse on_CloseWindowRequest(CloseWindowRequest& request) override {};
    void on_CreateWindowResponse(CreateWindowResponse& response) override;

    // clildren
    Window& window() { return m_window; }
    const Window& window() const { return m_window; }

    inline void invalidate_area(int x, int y, int width, int height)
    {
        m_connection.send_InvalidateRequest(InvalidateRequest(m_window.id(), 0, 0, m_width, m_height));
    }

private:
    Connection m_connection { Connection("/ext2/ws.socket", *this) };

    // Create window request properties
    int m_width {};
    int m_height {};
    String m_titile {};

    Window m_window;

    View* m_content_view {};
};

}