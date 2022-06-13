#include "DualTimerSP804.hpp"
#include <Hardware/x86/Port.hpp>
#include <Tasking/MemoryDescription/MemoryDescription.hpp>
#include <Tasking/MemoryDescription/SharedVMArea.hpp>

namespace Kernel::Devices {

DualTimerSP804 sp804;

using namespace Tasking;

constexpr auto sp804_first_timer_physical_address = 0x1c110000;
constexpr auto sp804_clock_frequency = 1000000;

void DualTimerSP804::install()
{
    auto sp804_area = kernel_memory_description.allocate_memory_area<SharedVMArea>(
        sizeof(SP804Registers),
        VM_READ | VM_WRITE,
        true);

    if (!sp804_area) {
        ASSERT_PANIC("[DualTimerSP804] Could not allocate sp804 vmarea");
    }

    auto sp804_registers_virtual = sp804_area.result()->vm_start();

    VMM::the().map_memory(
        sp804_registers_virtual,
        sp804_first_timer_physical_address,
        sizeof(SP804Registers),
        1);

    m_registers = reinterpret_cast<SP804Registers*>(sp804_registers_virtual);

    m_registers->load = sp804_clock_frequency / frequency();
    m_registers->control = sp804_timer_enable | sp804_timer_mode_periodic | sp804_timer_size_32bit | sp804_interrupt_enable;
}

void DualTimerSP804::handle_interrupt(Trapframe* tf)
{
    // Pass control to the generic InterruptTimer.
    // dispatch_callbacks(tf);
    Log() << "DualTimerSP804\n";
    m_registers->interrupt_clear = 1;
}

}