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
    auto uart_area = kernel_memory_description.allocate_memory_area<SharedVMArea>(
        sizeof(PL011Registers),
        VM_READ | VM_WRITE,
        true);

    if (!uart_area) {
        ASSERT_PANIC("[UART] Could not allocate uart vmarea");
    }

    auto uart_base_register_virtual = uart_area.result()->vm_start();

    VMM::the().map_memory(
        uart_base_register_virtual,
        uart0_physical_address,
        sizeof(PL011Registers),
        1);

    m_registers = reinterpret_cast<PL011Registers*>(uart_base_register_virtual);

    // Disable UART.
    m_registers->control = 0;

    // Set required baud rate.
    m_registers->integer_baud_rate = uart_frequency / (16 * uart_required_frequency) & 0xff;
    m_registers->fractional_baud_rate = ((uart_frequency * 4 / uart_required_frequency) >> 6) & 0x3f;

    // Enable FIFO.
    m_registers->line_control = pl011_enable_fifo | pl011_word_length_8_bits;

    // Enable interupts.
    auto reg = m_registers->interrupt_mask_set_clear;
    reg |= pl011_receive_interrupt_enable;
    m_registers->interrupt_mask_set_clear = reg;

    // Enable UART in transfer and receive modes.
    m_registers->control = pl011_uart_enable | pl011_transmit_enable | pl011_receive_enable;

    return DriverInstallationResult::Succeeded;
}

bool UartPL011::can_receive()
{
    return !(m_registers->flags & pl011_uart_busy || m_registers->flags & pl011_receive_fifo_empty);
}

char UartPL011::receive_byte()
{
    while (!can_receive()) { }
    return m_registers->data;
}

bool UartPL011::can_send()
{
    return !(m_registers->flags & pl011_uart_busy || m_registers->flags & pl011_transmit_fifo_full);
}

void UartPL011::send_byte(char byte)
{
    while (!can_send()) { }
    m_registers->data = byte;
}

void UartPL011::handle_interrupt(Trapframe* tf)
{
    auto masked_interrupt = m_registers->masked_interrupt_status;
    auto byte = receive_byte();
    Log() << "uart " << byte << "\n";
    m_registers->interrupt_clear = masked_interrupt;
}

}