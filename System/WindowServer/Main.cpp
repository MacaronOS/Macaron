#include "WindowServer.hpp"

#include <Libui/Application.hpp>

int main()
{
    auto wm = WindowServer();
    if (wm.initialize()) {
        wm.run();
    }

    return 0;
}