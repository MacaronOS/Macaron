#include "Uart.hpp"
#include <Libkernel/Assert.hpp>
#include <Memory/VMM/VMM.hpp>
#include <Tasking/MemoryDescription/MemoryDescription.hpp>
#include <Tasking/MemoryDescription/SharedVMArea.hpp>

namespace Kernel::Devices {

constexpr auto uart_base_register_physical = 0x1c090000;

using namespace Tasking;

UART uart;

DriverInstallationResult UART::try_install_console()
{
    auto uart_area = kernel_memory_description.allocate_memory_area<SharedVMArea>(
        sizeof(char),
        VM_READ | VM_WRITE,
        true);

    if (!uart_area) {
        ASSERT_PANIC("[UART] Could not allocate uart vmarea");
    }

    auto uart_base_register_virtual = uart_area.result()->vm_start();

    VMM::the().map_memory(
        uart_base_register_virtual,
        uart_base_register_physical,
        sizeof(char),
        1);

    m_base_register = reinterpret_cast<char*>(uart_base_register_virtual);

    return DriverInstallationResult::Succeeded;
}

bool UART::can_receive()
{
    return false;
}

char UART::receive_byte()
{
    return 0;
}

bool UART::can_send()
{
    return true;
}

void UART::send_byte(char byte)
{
    *m_base_register = byte;
}

}