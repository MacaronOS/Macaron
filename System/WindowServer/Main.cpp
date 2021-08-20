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

int main()
{
    auto wm = WindowServer();
    if (wm.initialize()) {
        wm.run();
    }

    return 0;
}