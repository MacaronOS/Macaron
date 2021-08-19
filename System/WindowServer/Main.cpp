#include "WindowServer.hpp"

#include <Libc/Malloc.hpp>
#include <Libc/Syscalls.hpp>

#include <Libsystem/Log.hpp>
#include <Libsystem/Syscalls.hpp>

#include <Libgraphics/Bitmap.hpp>
#include <Libgraphics/Color.hpp>
#include <Libui/Application.hpp>

#include <Wisterialib/posix/defines.hpp>

#include <Libipc/ClientConnection.hpp>
#include <Libipc/ServerConnection.hpp>

#include <Libui/WSProtocols/ClientConnection.hpp>
#include <Libui/WSProtocols/ServerConnection.hpp>

void run_demo_1()
{
    if (!fork()) {
        auto& app = UI::Application::the();
        app.set_intitial_window_width(240);
        app.set_intitial_window_height(180);
        app.set_intitial_window_titile("Application 1");
        app.run();
    }
}

void run_demo_2()
{
    if (!fork()) {
        auto& app = UI::Application::the();
        app.set_intitial_window_width(300);
        app.set_intitial_window_height(200);
        app.set_intitial_window_titile("Application 2");
        app.run();
    }
}

int main()
{
    run_demo_1();
    run_demo_2();

    auto wm = WindowServer();
    if (wm.initialize()) {
        wm.run();
    }

    return 0;
}