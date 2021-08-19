#include "WindowServer.hpp"

#include <libc/malloc.hpp>
#include <libc/syscalls.hpp>

#include <libsys/Log.hpp>
#include <libsys/syscalls.hpp>

#include <libgraphics/Bitmap.hpp>
#include <libgraphics/Color.hpp>
#include <libui/Application.hpp>

#include <wisterialib/posix/defines.hpp>

#include <libipc/ClientConnection.hpp>
#include <libipc/ServerConnection.hpp>

#include <libui/WSProtocols/ClientConnection.hpp>
#include <libui/WSProtocols/ServerConnection.hpp>

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