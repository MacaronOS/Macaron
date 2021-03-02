#include <libc/syscalls.h>

int main()
{   
    int p = fork();

    if (p) {
        for (int i = 0; i < 1000000; i++) {
            putc('a');
        }
    } else {
        execve("/ext2/apps/putb.app", 0, 0);
    }
    return 0;
}