#include "types.hpp"
#include "algo/String.hpp"

volatile uint16_t* vga_buffer = (uint16_t*)0xB8000;

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
    for (int row = 0; row < VGA_ROWS; row++) {
        for (int col = 0; col < VGA_COLS; col++) {
            vga_buffer[row * VGA_COLS + col] = ((uint16_t)term_color << 8) | ' ';
        }
    }
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
}

void term_print(const char* str)
{
    for (size_t i = 0; str[i] != '\0'; i++) {
        term_putc(str[i]);
    }
}

using kernel::algorithms::String;
void term_print(const String& str) {
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
