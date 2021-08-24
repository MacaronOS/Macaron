#include "WindowServer.hpp"

int main()
{
    auto wm = WindowServer();
    if (wm.initialize()) {
        wm.run();
    }

    return 0;
}