#pragma once

#include "Connection.hpp"
#include "Window.hpp"

namespace UI {

class Application : public ClientMessageReciever {
public:
    Application() = default;

    void set_intitial_window_width(int width) { m_width = width; }
    void set_intitial_window_height(int heigth) { m_height = heigth; }
    void set_intitial_window_titile(String titile) { m_titile = move(titile); }

    void run();

    void on_MousePressRequest(MousePressRequest& request) override { }
    void on_MouseMoveRequest(MouseMoveRequest& request) override {};
    CloseWindowResponse on_CloseWindowRequest(CloseWindowRequest& request) override {};
    void on_CreateWindowResponse(CreateWindowResponse& response) override;

    Window& window() { return m_window; }
    const Window& window() const { return m_window; }

    void invalidate_area(int x, int y, int width, int height);

private:
    Connection m_connection { Connection("/ext2/ws.socket", *this) };

    // Create window request properties
    int m_width {};
    int m_height {};
    String m_titile {};

    Window m_window;
};

}