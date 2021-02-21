#include <libc/syscalls.h>

int main()
{   
    int p = fork();

    if (p) {
        for (int i = 0; i < 1000000; i++) {
            putc('a');
        }
    } else {
        for (int i = 0; i < 1000000; i++) {
            putc('b');
        }
    }
    return 0;
}