#pragma once

#include "../Connection.hpp"
#include "../Content/Intent.hpp"
#include "../Events.hpp"

#include <Macaronlib/Vector.hpp>
#include <Macaronlib/List.hpp>

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
    virtual void on_ScreenSizeResponse(ScreenSizeResponse& response) override;
    virtual void on_BackRequest(BackRequest& request) override;

    inline void make_frameless() { m_frameless = true; }
    inline void push_activity_onto_the_stack(Activity* activity) { m_activity_stack.push_back(activity); }
    inline void run() { EventLoop::the().run(); }

    void invalidate_area(int x, int y, int width, int height);
    void create_window(int width, int height, const String& titile);
    void enqueue_on_window(int width, int height, const String& titile, Activity* activity);
    void ask_screen_size(const Function<void(int width, int height)>& callback);
    void set_position(Activity* activity, int left, int top);

    void register_new_activity(Activity* activity);

protected:
    bool m_frameless {};
    Vector<Activity*> m_activity_stack {};
    Connection m_connection { Connection("/ext2/ws.socket", *this) };
    Vector<Function<void(int width, int height)>> m_on_screen_size_callbacks {};

    List<Activity*> m_pending_on_window {};
};

}