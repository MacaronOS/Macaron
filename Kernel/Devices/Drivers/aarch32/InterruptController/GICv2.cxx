#include "GICv2.hpp"

#include <Hardware/Interrupts/aarch32/InterruptArchManager.hpp>
#include <Hardware/aarch32/CPU.hpp>
#include <Tasking/MemoryDescription/MemoryDescription.hpp>
#include <Tasking/MemoryDescription/SharedVMArea.hpp>

namespace Kernel::Devices {

using namespace Tasking;
using namespace Hardware;

GICv2 gicv2;

constexpr auto distributor_peripheral_offset = 0x1000;
constexpr auto cpu_interface_peripheral_offset = 0x2000;

constexpr auto distributor_enable = 0x1;
constexpr auto cpu_interface_enable = 0x1;

void GICv2::install()
{
    auto gicv2_distributor_registers_area = kernel_memory_description.allocate_memory_area<SharedVMArea>(
        sizeof(DistributorRegisters),
        VM_READ | VM_WRITE,
        true);

    if (!gicv2_distributor_registers_area) {
        ASSERT_PANIC("[GICv2] Could not allocate gicv2 vmarea");
    }

    auto gicv2_cpu_interface_registers_area = kernel_memory_description.allocate_memory_area<SharedVMArea>(
        sizeof(DistributorRegisters),
        VM_READ | VM_WRITE,
        true);

    if (!gicv2_cpu_interface_registers_area) {
        ASSERT_PANIC("[GICv2] Could not allocate gicv2 vmarea");
    }

    auto gicv2_distributor_registers_virtual = gicv2_distributor_registers_area.result()->vm_start();
    auto gicv2_cpu_interface_registers_virtual = gicv2_cpu_interface_registers_area.result()->vm_start();

    auto peripheral_base = read_peripheral_base();
    auto gicv2_distributor_registers_physical = peripheral_base + distributor_peripheral_offset;
    auto gicv2_cpu_interface_registers_physical = peripheral_base + cpu_interface_peripheral_offset;

    VMM::the().map_memory(
        gicv2_distributor_registers_virtual,
        gicv2_distributor_registers_physical,
        sizeof(DistributorRegisters),
        1);

    VMM::the().map_memory(
        gicv2_cpu_interface_registers_virtual,
        gicv2_cpu_interface_registers_physical,
        sizeof(CPUInterfaceRegisters),
        1);

    m_distributor_registers = reinterpret_cast<DistributorRegisters*>(gicv2_distributor_registers_virtual);
    m_cpu_interface_registers = reinterpret_cast<CPUInterfaceRegisters*>(gicv2_cpu_interface_registers_virtual);

    // Enabe the distributor.
    m_distributor_registers->distributor_control = distributor_enable;

    // Enable all interrupt priorities on the cpu interface.
    m_cpu_interface_registers->interrupt_priority_mask = 0xffff;

    // Enable the cpu interface.
    m_cpu_interface_registers->cpu_interface_control = cpu_interface_enable;

    InterruptArchManager::the().set_interrupt_controller(this);
    InterruptArchManager::the().setup_interrupts();

    CPU::enable_interrupts();
}

void GICv2::enable_interrupt(uint32_t irq, InterruptType type)
{
    // Use 128 priporiry by default.
    auto priority = 128;
    // Dispatch to the first cpu by default.
    auto cpu_mask = 1;

    auto iprreg = irq / 4;
    auto iprbit = (irq % 4) * 8;
    m_distributor_registers->interrupt_priority[iprreg] |= (priority << iprbit);

    auto icfgreg = irq / 16;
    auto icfgbit = (irq % 16) * 2;
    if (type == InterruptType::EdgeTriggered) {
        m_distributor_registers->interrupt_configuration_registers[icfgreg] |= (0b10 << icfgbit);
    } else {
        m_distributor_registers->interrupt_configuration_registers[icfgreg] |= ~(uint32_t)(0b11 << icfgbit);
    }

    auto iprctrgtreg = irq / 4;
    auto iprctrgtrbit = (irq % 4) * 8;
    m_distributor_registers->interrupt_processor_targets[iprctrgtreg] |= (cpu_mask << iprctrgtrbit);

    auto isenreg = irq / 32;
    auto isenbit = irq % 32;
    m_distributor_registers->interrupt_set_enable[isenreg] |= (1 << isenbit);
}

uint32_t GICv2::acknowledge_interrupt()
{
    return m_cpu_interface_registers->interrupt_acknowledge & 0x1ff;
}

void GICv2::end_of_interrupt(uint32_t intno)
{
    m_cpu_interface_registers->end_of_interrupt = intno & 0x1ff;
}

}