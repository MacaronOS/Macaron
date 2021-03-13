#include <libc/syscalls.h>

int main()
{   
    int p = fork();

    if (p) {
        while (1)
        {
        }
        
    } else {
        execve("/ext2/apps/gui.app", 0, 0);
    }
    return 0;
}