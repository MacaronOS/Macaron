#include "VgaTUI.hpp"
#include <Hardware/x86/Port.hpp>

namespace Kernel::Devices {

VgaTUI vga_tui;

namespace VGA {
    static volatile uint16_t* text_buffer = (uint16_t*)0xC00B8000;

    static constexpr int text_colomns = 80;
    static constexpr int text_rows = 25;

    static int column = 0;
    static int row = 0;
    static uint8_t color = 0x0F;

    static inline void clear_screen()
    {
        column = row = 0;
        for (size_t row = 0; row < text_rows; row++) {
            for (size_t col = 0; col < text_colomns; col++) {
                text_buffer[row * text_colomns + col] = ((uint16_t)color << 8) | ' ';
            }
        }
    }

    static inline void enable_cursor()
    {
        outb(0x3D4, 0x0A);
        outb(0x3D5, (inb(0x3D5) & 0xC0) | 0);

        outb(0x3D4, 0x0B);
        outb(0x3D5, (inb(0x3D5) & 0xE0) | 15);
    }

    static inline void update_cursor()
    {
        uint16_t pos = row * text_colomns + column;

        outb(0x3D4, 0x0F);
        outb(0x3D5, (uint8_t)(pos & 0xFF));
        outb(0x3D4, 0x0E);
        outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
    }

    static inline int get_index()
    {
        return row * text_colomns + column;
    }

    static inline void next_row()
    {
        column = 0;
        row++;
        if (row >= text_rows) {
            row = 0;
            column = 0;
            clear_screen();
        }
    }

    static inline void increment_position()
    {
        column++;
        if (column >= text_colomns) {
            next_row();
        }
    }
}

DriverInstallationResult VgaTUI::try_install()
{
    VGA::clear_screen();
    VGA::enable_cursor();
    return DriverInstallationResult::Succeeded;
}

bool VgaTUI::can_receive()
{
    return false;
}

char VgaTUI::receive_byte()
{
    return '\0';
}

bool VgaTUI::can_send()
{
    return true;
}

void VgaTUI::send_byte(char byte)
{
    switch (byte) {
    case '\n':
        VGA::next_row();
        break;

    default:
        VGA::text_buffer[VGA::get_index()] = ((uint16_t)VGA::color << 8) | byte;
        VGA::increment_position();
        break;
    }

    VGA::update_cursor();
}

}