#include "KeyboardPL050.hpp"

namespace Kernel::Devices {

KeyboardPL050 kbd;

constexpr auto keyboard_kmi_base = 0x1c060000;
constexpr auto keyboard_scan_code_set = 0xf0;
constexpr auto keyboard_enable_scanning = 0xf4;

DriverInstallationResult KeyboardPL050::try_install()
{
    if (m_controller.initialize(keyboard_kmi_base) != PL050Status::Initialized) {
        ASSERT_PANIC("[KeyboardPL050] Could not initialize mouse PL050Controller");
    }

    m_controller.send_command_and_data(keyboard_scan_code_set, 1);
    m_controller.send_command(keyboard_enable_scanning);
    m_controller.enable_with_interrupts();

    return DriverInstallationResult::Succeeded;
}

void KeyboardPL050::handle_interrupt(Trapframe* tf)
{
    uint32_t data = m_controller.receive_data();
    Log() << "KBD " << data << "\n";
}

}