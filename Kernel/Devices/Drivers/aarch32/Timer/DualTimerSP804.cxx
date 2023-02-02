#include "DualTimerSP804.hpp"
#include <Hardware/x86/Port.hpp>
#include <Tasking/MemoryDescription/MemoryDescription.hpp>
#include <Tasking/MemoryDescription/SharedVMArea.hpp>

namespace Kernel::Devices {

DualTimerSP804 sp804;

using namespace Tasking;

constexpr auto sp804_clock_frequency = 1000000;

void DualTimerSP804::install()
{
    m_registers_mapper.map_all_registers();
    auto registers = m_registers_mapper.get_register_mask<0>();

    registers->load = sp804_clock_frequency / frequency();
    registers->control = sp804_timer_enable | sp804_timer_mode_periodic | sp804_timer_size_32bit | sp804_interrupt_enable;
}

void DualTimerSP804::handle_interrupt(Trapframe* tf)
{
    // Pass control to the generic InterruptTimer.
    dispatch_callbacks(tf);
    // Log() << "DualTimerSP804\n";
    auto registers = m_registers_mapper.get_register_mask<0>();
    registers->interrupt_clear = 1;
}

}