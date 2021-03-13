#include <libc/syscalls.h>

int main()
{
    int fd = open("/dev/bga", 1, 1);
    if (fd < 0) {
        return;
    }

    int res = mmap(0, 1024 * 768 * 4 * 2, PROT_NONE, MAP_SHARED, fd, 0);
    if (res < 0) {
        return;
    }

    unsigned int* pixels = (unsigned int*)res;
    for (int i = 0; i < 1024 * 768; i++) {
        pixels[i] = 0x0ffff00;
    }
    
    return 0;
}