#ifndef MISTIX_KERNEL_MONITOR_H
#define MISTIX_KERNEL_MONITOR_H

#include "../types.hpp"
#include "algo/String.hpp"

void term_init();

void term_print(const char* str);

using kernel::algorithms::String;
void term_print(const String&);

void term_printn(int64_t numb, uint32_t s);

void term_printd(int64_t);

void term_putc(char);

#endif // MISTIX_KERNEL_MONITOR_H