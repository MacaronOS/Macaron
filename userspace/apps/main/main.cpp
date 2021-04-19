#include <libc/syscalls.hpp>
#include <wisterialib/String.hpp>
#include <libsys/Log.hpp>

int main()
{   
    Log << String("main") << endl;
    int p = fork();

    if (p) {
        while (1)
        {
        }
        
    } else {
        execve("/ext2/apps/window_server.app", 0, 0);
    }
    return 0;
}