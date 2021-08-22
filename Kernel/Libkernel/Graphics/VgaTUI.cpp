#include <Hardware/Port.hpp>
#include <Libkernel/Graphics/VgaTUI.hpp>

#include <Macaronlib/String.hpp>
#include <Macaronlib/Common.hpp>

static volatile uint16_t* vga_buffer = (uint16_t*)0xC00B8000;

static constexpr int VGA_COLS = 80;
static constexpr int VGA_ROWS = 25;

static int term_col = 0;
static int term_row = 0;
static uint8_t term_color = 0x0F;

static void enable_cursor()
{
    outb(0x3D4, 0x0A);
    outb(0x3D5, (inb(0x3D5) & 0xC0) | 0);

    outb(0x3D4, 0x0B);
    outb(0x3D5, (inb(0x3D5) & 0xE0) | 15);
}

static void update_cursor()
{
    uint16_t pos = term_row * VGA_COLS + term_col;

    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

static int get_vga_index()
{
    return term_row * VGA_COLS + term_col;
}

void VgaTUI::Initialize()
{
    term_col = term_row = 0;
    for (int row = 0; row < VGA_ROWS; row++) {
        for (int col = 0; col < VGA_COLS; col++) {
            vga_buffer[row * VGA_COLS + col] = ((uint16_t)term_color << 8) | ' ';
        }
    }
    enable_cursor();
}

void inc_vga_pos()
{
    term_col++;
    if (term_col >= VGA_COLS) {
        term_col = 0;
        term_row++;
    }
    if (term_row >= VGA_ROWS) {
        term_row = 0;
        term_col = 0;
        VgaTUI::Initialize();
    }
}

void VgaTUI::DecreaseCursor()
{
    term_col--;
    if (term_col == -1) {
        term_col = VGA_COLS - 1;
        term_row--;
    }
    if (term_row == -1) {
        term_row = 0;
    }

    vga_buffer[get_vga_index()] = ((uint16_t)term_color << 8) | '\0';
    update_cursor();
}

void VgaTUI::Putc(char c)
{
    switch (c) {
    case '\n':
        term_col = 0;
        term_row++;
        break;

    default:
        vga_buffer[get_vga_index()] = ((uint16_t)term_color << 8) | c;
        inc_vga_pos();
        break;
    }

    update_cursor();
}

void VgaTUI::Print(const char* str)
{
    for (size_t i = 0; str[i] != '\0'; i++) {
        VgaTUI::Putc(str[i]);
    }
}

void VgaTUI::Print(const String& str)
{
    for (size_t i = 0; i < str.size(); i++) {
        VgaTUI::Putc(str[i]);
    }
}

void VgaTUI::Printn(int64_t numb, uint32_t s)
{
    bool negative = 0;

    if (numb < 0) {
        numb *= -1;
        negative = 1;
    }

    char buffer[sizeof(uint32_t) * 8 + 2];
    int pos = 0;

    do {
        buffer[pos++] = numb % s + '0';
        numb /= s;
    } while (numb);

    if (negative) {
        VgaTUI::Putc('-');
    }

    while (pos) {
        VgaTUI::Putc((char)buffer[pos - 1]);
        pos--;
    }
}

void VgaTUI::Printd(int64_t numb)
{
    VgaTUI::Printn(numb, 10);
}
