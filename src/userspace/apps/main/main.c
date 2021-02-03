#include <libc/syscalls.h>

int main()
{
    for (int i = 0; i < 1000000; i++) {
        volatile char a = 'a';
        putc(a);
    }
    return 0;
}