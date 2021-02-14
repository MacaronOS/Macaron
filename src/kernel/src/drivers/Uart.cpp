#include "Uart.hpp"

#include <hardware/port.hpp>
#include <monitor.hpp>

namespace kernel::drivers {

bool Uart::install()
{
    write(Register::Interrupt, 0x00);
    write(Register::LineControl, 0x80);
    write(Register::Data, 0x03);
    write(Register::Interrupt, 0x00);
    write(Register::LineControl, 0x03);
    write(Register::Control, 0xC7);
    write(Register::ModemControl, 0x0B);
    write(Register::ModemControl, 0x1E);

    write(Register::Data, 0xAE);
    if (read(Register::Data) != 0xAE) {
        return false;
    }

    write(Register::ModemControl, 0x0F);
    return true;
}

void Uart::send(char byte)
{
    while (!can_send()) { }
    write(Register::Data, byte);
}

char Uart::recieve()
{
    while (!can_receive()) { }
    return read(Register::Data);
}

inline void Uart::write(Uart::Register reg, char byte)
{
    outb(static_cast<uint16_t>(reg), byte);
}

inline uint8_t Uart::read(Uart::Register reg)
{
    return inb(static_cast<uint16_t>(reg));
}

inline bool Uart::can_send()
{
    return read(Register::LineStatus) & 0x20;
}
inline bool Uart::can_receive()
{
    return read(Register::LineStatus) & 1;
}

}
