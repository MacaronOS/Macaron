#include <libc/syscalls.hpp>

#include <wisterialib/posix/defines.hpp>

int main()
{
    void* mem = mmap(0, 4096, 0, MAP_ANONYMOUS, 0, 0);
    if (mem == (void*)-1) {
        return 0;
    }
    *(char*)mem = 'b';

    for (int i = 0; i < 1000000; i++) {
        putc(*(char*)mem);
    }
    return 0;
}