#include <libc/syscalls.h>

int main()
{   
    int mem = mmap(0, 4096, 0, MAP_ANONYMOUS, 0, 0);
    if (mem < 0) {
        return 0;
    }
    *(char*)mem = 'b';

    for (int i = 0; i < 1000000; i++) {
        putc(*(char*)mem);
    }
    return 0;
}