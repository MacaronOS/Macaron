#include "WindowServer.hpp"

#include <libc/malloc.hpp>
#include <libc/syscalls.hpp>

#include <libsys/Log.hpp>
#include <libsys/syscalls.hpp>

#include <libgraphics/Application.hpp>
#include <libgraphics/Bitmap.hpp>
#include <libgraphics/Color.hpp>

#include <wisterialib/posix/defines.hpp>

#include <libipc/ClientConnection.hpp>
#include <libipc/ServerConnection.hpp>

#include <libui/WSProtocols/ClientConnection.hpp>
#include <libui/WSProtocols/ServerConnection.hpp>

class ClientReciever : public UI::Protocols::ClientMessageReciever {
public:
    virtual void on_MousePressRequest(UI::Protocols::MousePressRequest& request) override { }
    virtual void on_MouseMoveRequest(UI::Protocols::MouseMoveRequest& request) override { }
    virtual UI::Protocols::CloseWindowResponse on_CloseWindowRequest(UI::Protocols::CloseWindowRequest& request) override { }

    void on_CreateWindowResponse(UI::Protocols::CreateWindowResponse& response) override
    {
        Log << "recieved create window response " << endl;
        Log << "window id " << response.window_id() << endl;
    }
};

class ServerReciever : public UI::Protocols::ServerMessageReciever {
public:
    UI::Protocols::CreateWindowResponse on_CreateWindowRequest(UI::Protocols::CreateWindowRequest& request) override
    {
        Log << "recieved create window request " << endl;
        Log << "width " << request.widht() << endl;
        Log << "height " << request.height() << endl;
        Log << "title " << request.titile() << endl;
        return UI::Protocols::CreateWindowResponse(request.height());
    }

    virtual void on_CloseWindowResponse(UI::Protocols::CloseWindowResponse& response) override { }
};

void run_demo()
{
    // if (!fork()) {
    //     auto app = Application();
    //     size_t cnt = 0;

    //     while (true) {
    //         for (int red = 0; red < 0xff; red += 10) {
    //             for (int green = 0; green < 0xff; green += 20) {
    //                 for (int blue = 0; blue < 0xff; blue += 30) {

    //                     for (size_t y = 0; y < app.window().height(); y++) {
    //                         for (size_t x = 0; x < app.window().width(); x++) {
    //                             app.window().buffer()[y][x] = Graphics::Color(red, green, blue);
    //                             if (cnt == 0) {
    //                                 app.invalidate_area(0, 0, app.window().width(), app.window().height());
    //                             }
    //                             cnt++;
    //                             if (cnt % 1000000 == 0) {
    //                                 app.invalidate_area(0, 0, app.window().width() / 2, app.window().height() / 2);
    //                             }
    //                         }
    //                     }

    //                     sched_yield();
    //                 }
    //             }
    //         }
    //     }
    // }

    if (!fork()) {
        Log << "INIT CLIENT" << endl;
        ClientReciever reciever;
        auto client_connection = UI::Protocols::ClientConnection("/ext2/ws.socket", reciever);
        client_connection.send_CreateWindowRequest(UI::Protocols::CreateWindowRequest(50, 200, "window titile"));
        client_connection.send_CreateWindowRequest(UI::Protocols::CreateWindowRequest(101, 201, "window titile 1"));
        client_connection.send_CreateWindowRequest(UI::Protocols::CreateWindowRequest(102, 202, "window titile 2"));

        while (true) {
            client_connection.process_messages();
        }

    } else {
        Log << "INIT SERVER" << endl;
        ServerReciever reciever;
        auto server_connection = UI::Protocols::ServerConnection("/ext2/ws.socket", reciever);
        while (true) {
            server_connection.process_messages();
        }
    }
}

int main()
{
    // run_demo();
    run_demo();

    // auto wm = WindowServer();
    // if (wm.initialize()) {
        // wm.run();
    // }

    return 0;
}