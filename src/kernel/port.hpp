#ifndef MISTIX_KERNEL_PORT_H
#define MISTIX_KERNEL_PORT_H

#include "types.hpp"

void outb(uint16_t port, uint8_t value);
void outw(uint16_t port, uint16_t value);
uint8_t inb(uint16_t port);
uint16_t inw(uint16_t port);

#endif // MISTIX_KERNEL_PORT_H