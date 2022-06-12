#include "MousePL050.hpp"

namespace Kernel::Devices {

MousePL050 mouse;

constexpr auto mouse_kmi_base = 0x1c070000;
constexpr auto mouse_enable_data_reporting = 0xf4;
constexpr auto mouse_set_defaults = 0xf6;

DriverInstallationResult MousePL050::try_install()
{
    if (m_controller.initialize(mouse_kmi_base) != PL050Status::Initialized) {
        ASSERT_PANIC("[MousePL050] Could not initialize mouse PL050Controller");
    }

    m_controller.send_command(mouse_set_defaults);
    m_controller.send_command(mouse_enable_data_reporting);
    m_controller.enable_with_interrupts();

    return DriverInstallationResult::Succeeded;
}

void MousePL050::handle_interrupt(Trapframe* tf)
{
    uint32_t data = m_controller.receive_data();
    Log() << "Mouse " << data << "\n";
}

}