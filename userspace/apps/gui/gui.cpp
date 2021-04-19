#include <libc/syscalls.hpp>
#include <libc/malloc.hpp>

#include <libsys/Log.hpp>
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

    Log << "PIXELS " << (uint32_t)pixels << endl;

    for (int i = 0; i < 1024 * 768; i++) {
        pixels[i] = 0x0ffff00;
    }

    ioctl(fd, BGA_SWAP_BUFFERS);

    for (int i = 0; i < 1024 * 768; i++) {
        pixels[i] = 0xfffffff;
    }

    ioctl(fd, BGA_SWAP_BUFFERS);

    for (int i = 0; i < 1024 * 768; i++) {
        pixels[i + 1024 * 768] = 0x0000ffff;
    }

    ioctl(fd, BGA_SWAP_BUFFERS);
    ioctl(fd, BGA_SWAP_BUFFERS);

    if (!fork()) {
        uint32_t* mem = (uint32_t * )create_shared_buffer(1010).mem;

        while (1) {
            for (uint32_t pixel = 0 ; pixel < 0xfffffff ; pixel++) {
                for (int i = 0; i < 1024 * 768; i++) {
                    pixels[i] = pixel;
                }
                Log << pixel << " " << getpid() << endl;
                mem[200] = pixel;
            }
        }
    } else {
        for (size_t i = 0 ; i < 100 ; i++) {
            Log << "waiting for process 1" << endl;
        }
        uint32_t* mem = (uint32_t* )get_shared_buffer(0);

        while (1) {
            Log << mem[200] << " " << getpid() << endl;
        }
    }


    return 0;
}