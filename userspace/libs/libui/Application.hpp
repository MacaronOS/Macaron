#pragma once

#include "Connection.hpp"
#include "Window.hpp"

namespace UI {

class Application : public ClientMessageReciever {
public:
    Application() = default;
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
    Window m_window;
};

}