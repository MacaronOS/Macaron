#include "BMPLoader.hpp"

#include <Libc/Syscalls.hpp>
#include <Libsystem/Log.hpp>
#include <Macaronlib/ABI/Syscalls.hpp>
#include <Macaronlib/Memory.hpp>
namespace Graphics::BMPLoader {

struct [[gnu::packed]] BMPFileHeader {
    uint16_t file_type;
    uint32_t file_size;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t offset_data;
};

struct [[gnu::packed]] BMPInfoHeader {
    uint32_t size;
    int32_t width;
    int32_t height;
    uint16_t planes;
    uint16_t bit_count;
    uint32_t compression;
    uint32_t size_image;
    int32_t x_pixels_per_meter;
    int32_t y_pixels_per_meter;
    uint32_t colors_used;
    uint32_t colors_important;
};

struct [[gnu::packed]] BMPColorHeader {
    uint32_t red_mask;
    uint32_t green_mask;
    uint32_t blue_mask;
    uint32_t alpha_mask;
    uint32_t color_space_type;
    uint32_t unused[16];
};

Graphics::Bitmap load(const String& path)
{
    int fd = open(path.c_str(), 1, 1);

    if (fd < 0) {
        return {};
    }

    BMPFileHeader file_header;
    BMPInfoHeader info_header;
    BMPColorHeader color_header;

    read(fd, &file_header, sizeof(BMPFileHeader));
    if (file_header.file_type != 0x4D42) {
        return {};
    }

    read(fd, &info_header, sizeof(BMPInfoHeader));

    if (info_header.bit_count == 32) {
        if (info_header.size >= (sizeof(BMPInfoHeader) + sizeof(BMPColorHeader))) {
            read(fd, &color_header, sizeof(BMPColorHeader));
        }
    }

    lseek(fd, file_header.offset_data, SEEK_SET);

    auto height = info_header.height;
    auto widht = info_header.width;

    auto colors = (Graphics::Color*)malloc(4 * widht * height);

    read(fd, colors, widht * height * 4);

    auto colors_flipped = (Graphics::Color*)malloc(4 * widht * height);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < widht; x++) {
            colors_flipped[y * widht + x] = colors[(height - y) * widht + x];
        }
    }

    free(colors);

    return Graphics::Bitmap(colors_flipped, widht, height);
}

}