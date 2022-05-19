#include "Uart.hpp"
#include <Hardware/x86/Port.hpp>
#include <Libkernel/Assert.hpp>

namespace Kernel::Devices {

UART uart;

constexpr uint16_t UART_COM1 = 0x3f8;

enum class UARTRegister {
    Data = UART_COM1 + 0,
    Interrupt,
    Control,
    LineControl,
    ModemControl,
    LineStatus,
    ModemStatus,
    Scratch,
};

static inline void uart_write(UARTRegister reg, char byte)
{
    outb(static_cast<uint16_t>(reg), byte);
};

static inline auto uart_read(UARTRegister reg)
{
    return inb(static_cast<uint16_t>(reg));
}

DriverInstallationResult UART::try_install()
{
    uart_write(UARTRegister::Interrupt, 0x00);
    uart_write(UARTRegister::LineControl, 0x80);
    uart_write(UARTRegister::Data, 0x03);
    uart_write(UARTRegister::Interrupt, 0x00);
    uart_write(UARTRegister::LineControl, 0x03);
    uart_write(UARTRegister::Control, 0xC7);
    uart_write(UARTRegister::ModemControl, 0x0B);
    uart_write(UARTRegister::ModemControl, 0x1E);

    uart_write(UARTRegister::Data, 0xAE);
    if (uart_read(UARTRegister::Data) != 0xAE) {
        ASSERT_PANIC("[UART] Could not install driver");
        return DriverInstallationResult::Failed;
    }

    uart_write(UARTRegister::ModemControl, 0x0F);
    return DriverInstallationResult::Succeeded;
}

bool UART::can_receive()
{
    return uart_read(UARTRegister::LineStatus) & 1;
}

char UART::receive_byte()
{
    while (!can_receive()) { }
    return uart_read(UARTRegister::Data);
}

bool UART::can_send()
{
    return uart_read(UARTRegister::LineStatus) & 0x20;
}

void UART::send_byte(char byte)
{
    while (!can_send()) { }
    uart_write(UARTRegister::Data, byte);
}

}