#include <libc/syscalls.h>

extern int putc(char c);

int main()
{
    while (1) {
        volatile char a = 'a';
        putc(a);
    }
    return 0;
}