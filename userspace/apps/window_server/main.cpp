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

void run_demo()
{
    if (!fork()) {
        UI::Application app {};
        app.run();
    }
}

int main()
{
    run_demo();
    run_demo();

    auto wm = WindowServer();
    if (wm.initialize()) {
        wm.run();
    }

    return 0;
}