#include "monitor.hpp"

#include <wisterialib/String.hpp>
#include "hardware/port.hpp"
#include <wisterialib/common.hpp>

volatile uint16_t* vga_buffer = (uint16_t*)0xC00B8000;

const int VGA_COLS = 80;
const int VGA_ROWS = 25;

int term_col = 0;
int term_row = 0;
uint8_t term_color = 0x0F;

int get_vga_index()
{
    return term_row * VGA_COLS + term_col;
}

void term_init()
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
        term_init();
    }
}

void dec_vga_pos()
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

void term_putc(char c)
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

void term_print(const char* str)
{
    for (size_t i = 0; str[i] != '\0'; i++) {
        term_putc(str[i]);
    }
}

void term_print(const String& str)
{
    for (size_t i = 0; i < str.size(); i++) {
        term_putc(str[i]);
    }
}

void term_printn(int64_t numb, uint32_t s)
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
        term_putc('-');
    }

    while (pos) {
        term_putc((char)buffer[pos - 1]);
        pos--;
    }
}

void term_printd(int64_t numb)
{
    term_printn(numb, 10);
}

void enable_cursor()
{
    outb(0x3D4, 0x0A);
    outb(0x3D5, (inb(0x3D5) & 0xC0) | 0);

    outb(0x3D4, 0x0B);
    outb(0x3D5, (inb(0x3D5) & 0xE0) | 15);
}

void update_cursor()
{
    uint16_t pos = term_row * VGA_COLS + term_col;

    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}
