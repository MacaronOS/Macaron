#include <libc/syscalls.h>

int main()
{
    int fd = open("/dev/bga", 1, 1);
    if (fd < 0) {
        return 0;
    }

    volatile unsigned int* pixels = mmap(0, 1024 * 768 * 4 * 2, PROT_NONE, MAP_SHARED, fd, 0);
    if ((pixels == (void *) -1)) {
        return 0;
    }

    for (int i = 0; i < 1024 * 768; i++) {
        pixels[i] = 0x0ffff00;
    }
    
    return 0;
}