#include "FontLoader.hpp"
#include "../BMP/BMPLoader.hpp"
#include <Libc/Syscalls.hpp>
#include <Libsystem/Log.hpp>
#include <Wisterialib/ABI/Syscalls.hpp>

namespace Graphics::FontLoader {

struct [[gnu::packed]] Header {
    uint8_t identifier[3];
    uint8_t version;
};

struct [[gnu::packed]] InfoBlockDescription {
    uint8_t identifier;
    uint32_t size;
};

struct [[gnu::packed]] BlockInfo {
    int16_t font_size;
    uint8_t bit_field;
    uint8_t char_set;
    uint16_t stretch_h;
    uint8_t aa;
    uint8_t padding_up;
    uint8_t padding_right;
    uint8_t padding_down;
    uint8_t padding_left;
    uint8_t spacing_horiz;
    uint8_t spacing_vert;
    uint8_t outline;
    char font_name;
};

struct [[gnu::packed]] BlockCommon {
    uint16_t line_height;
    uint16_t base;
    uint16_t scale_w;
    uint16_t scale_h;
    uint16_t pages;
    uint8_t bit_field;
    uint8_t alpha_chnl;
    uint8_t red_chnl;
    uint8_t green_chnl;
    uint8_t blue_chnl;
};

struct [[gnu::packed]] KerningPair {
    uint32_t first;
    uint32_t second;
    int16_t amount;
};

struct [[gnu::packed]] Char {
    uint32_t id;
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;
    int16_t xoffset;
    int16_t yoffset;
    int16_t xadvantage;
    uint8_t page;
    uint8_t chnl;
};

BitmapFont load(const String& binary_description_path, const String& texture_path)
{
    int fd = open(binary_description_path.cstr(), 1, 1);
    if (fd < 0) {
        return {};
    }

    Header header;

    read(fd, &header, sizeof(Header));
    if (header.identifier[0] != 66 || header.identifier[1] != 77 || header.identifier[2] != 70) {
        return {};
    }

    BitmapFont font;

    for (size_t _ = 0; _ < 5; _++) {
        InfoBlockDescription info_block_description;
        read(fd, &info_block_description, sizeof(InfoBlockDescription));
        Vector<uint8_t> data(info_block_description.size);
        read(fd, data.data(), info_block_description.size);

        if (info_block_description.identifier == 1) {
            auto block_indo = (BlockInfo*)(data.data());

            font.name = String(&block_indo->font_name);

        } else if (info_block_description.identifier == 2) {
            auto block_common = (BlockCommon*)(data.data());

            font.line_height = block_common->line_height;
            font.base = block_common->base;
            font.width = block_common->scale_w;
            font.height = block_common->scale_h;

        } else if (info_block_description.identifier == 3) {
            // multiple pages are ignored for now
            continue;

        } else if (info_block_description.identifier == 4) {
            auto chars = (Char*)(data.data());
            for (size_t at = 0; at < data.size() / sizeof(Char); at++) {
                auto& cur_char = chars[at];

                font.chars[cur_char.id].x = cur_char.x;
                font.chars[cur_char.id].y = cur_char.y;
                font.chars[cur_char.id].width = cur_char.width;
                font.chars[cur_char.id].height = cur_char.height;
                font.chars[cur_char.id].xoffset = cur_char.xoffset;
                font.chars[cur_char.id].yoffset = cur_char.yoffset;
                font.chars[cur_char.id].xadvantage = cur_char.xadvantage;
            }

        } else if (info_block_description.identifier == 5) {
            auto kerning_pairs = (KerningPair*)(data.data());
            for (size_t at = 0; at < data.size() / sizeof(KerningPair); at++) {
                auto& kerning_pair = kerning_pairs[at];
                font.kerning[kerning_pair.first][kerning_pair.second] = kerning_pair.amount;
            }

        } else {
            break;
        }
    }

    font.texture = BMPLoader::load(texture_path);

    return font;
}

}