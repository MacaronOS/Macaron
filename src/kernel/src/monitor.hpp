#ifndef MISTIX_KERNEL_MONITOR_H
#define MISTIX_KERNEL_MONITOR_H

#include "types.hpp"
#include "algo/String.hpp"

void term_init();

void term_print(const char* str);

using kernel::String;
void term_print(const String&);

void term_printn(int64_t numb, uint32_t s);

void term_printd(int64_t);

void term_putc(char);

void dec_vga_pos();

void enable_cursor();

void update_cursor();

#endif // MISTIX_KERNEL_MONITOR_H