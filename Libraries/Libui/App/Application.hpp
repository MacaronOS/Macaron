#pragma once

#include "../Connection.hpp"
#include "../Events.hpp"

#include <Macaronlib/Vector.hpp>

namespace UI {

class Activity;

class Application : public ClientMessageReciever {
public:
    static Application& the()
    {
        static Application the;
        return the;
    }

    inline Connection& connection() { return m_connection; }
    inline const Vector<Activity*>& activity_stack() const { return m_activity_stack; }

    virtual void on_MouseClickRequest(MouseClickRequest& request) override;
    virtual void on_MouseMoveRequest(MouseMoveRequest& request) override;
    virtual CloseWindowResponse on_CloseWindowRequest(CloseWindowRequest& request) override { }
    virtual void on_CreateWindowResponse(CreateWindowResponse& response) override;

    inline void push_activity_onto_the_stack(Activity* activity) { m_activity_stack.push_back(activity); }
    inline void run() { EventLoop::the().run(); }

    void invalidate_area(int x, int y, int width, int height);

protected:
    Vector<Activity*> m_activity_stack {};
    Connection m_connection { Connection("/ext2/ws.socket", *this) };
};

}