#include <libc/syscalls.hpp>
#include <libc/malloc.hpp>

#include <libsys/syscalls.hpp>

#include <wisterialib/posix/defines.hpp>

int main()
{
    int fd = open("/dev/bga", 1, 1);
    if (fd < 0) {
        return 0;
    }

    volatile unsigned int* pixels = (unsigned int*)mmap(0, 1024 * 768 * 4 * 2, PROT_NONE, MAP_SHARED, fd, 0);
    if (pixels == (void*)-1) {
        return 0;
    }

    for (int i = 0; i < 1024 * 768; i++) {
        pixels[i] = 0x0ffff00;
    }

    if (fork()) {
        while (1) {
            for (uint32_t pixel = 0 ; pixel < 0xfffffff ; pixel++) {
                for (int i = 0; i < 1024 * 768; i++) {
                    pixels[i] = pixel;
                }
            }
        }
    } else {
        while (1) {
            write_string("testing");
        }
    }

    return 0;
}