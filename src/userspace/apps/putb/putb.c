#include <libc/syscalls.h>

int main()
{   
    for (int i = 0; i < 1000000; i++) {
        putc('b');
    }
    return 0;
}