#include "UartPL011.hpp"
#include "PL011Registers.hpp"

#include <Libkernel/Assert.hpp>
#include <Memory/VMM/VMM.hpp>
#include <Tasking/MemoryDescription/MemoryDescription.hpp>
#include <Tasking/MemoryDescription/SharedVMArea.hpp>

namespace Kernel::Devices {

using namespace Tasking;

UartPL011 uart;

constexpr auto uart0_physical_address = 0x1c090000;
constexpr auto uart_frequency = 24000000;
constexpr auto uart_required_frequency = 115200;

DriverInstallationResult UartPL011::try_install_console()
{
    m_registers_mapper.map_all_registers();
    auto registers = m_registers_mapper.get_register_mask<0>();

    // Disable UART.
    registers->control = 0;

    // Set the required baud rate.
    registers->integer_baud_rate = uart_frequency / (16 * uart_required_frequency) & 0xff;
    registers->fractional_baud_rate = ((uart_frequency * 4 / uart_required_frequency) >> 6) & 0x3f;

    // Enable FIFO.
    registers->line_control = pl011_enable_fifo | pl011_word_length_8_bits;

    // Enable interupts.
    auto reg = registers->interrupt_mask_set_clear;
    reg |= pl011_receive_interrupt_enable;
    registers->interrupt_mask_set_clear = reg;

    // Enable UART in transfer and receive modes.
    registers->control = pl011_uart_enable | pl011_transmit_enable | pl011_receive_enable;

    return DriverInstallationResult::Succeeded;
}

bool UartPL011::can_receive()
{
    auto registers = m_registers_mapper.get_register_mask<0>();
    return !(registers->flags & pl011_uart_busy || registers->flags & pl011_receive_fifo_empty);
}

char UartPL011::receive_byte()
{
    auto registers = m_registers_mapper.get_register_mask<0>();
    while (!can_receive()) { }
    return registers->data;
}

bool UartPL011::can_send()
{
    auto registers = m_registers_mapper.get_register_mask<0>();
    return !(registers->flags & pl011_uart_busy || registers->flags & pl011_transmit_fifo_full);
}

void UartPL011::send_byte(char byte)
{
    auto registers = m_registers_mapper.get_register_mask<0>();
    while (!can_send()) { }
    registers->data = byte;
}

void UartPL011::handle_interrupt(Trapframe* tf)
{
    auto registers = m_registers_mapper.get_register_mask<0>();
    auto masked_interrupt = registers->masked_interrupt_status;
    auto byte = receive_byte();
    Log() << "uart " << byte << "\n";
    registers->interrupt_clear = masked_interrupt;
}

}