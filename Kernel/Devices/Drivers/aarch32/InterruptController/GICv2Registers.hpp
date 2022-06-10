#pragma once

#include <Macaronlib/Common.hpp>

namespace Kernel::Devices {

// GICv2 registers description are taken from here:
// https://developer.arm.com/documentation/ihi0048/latest

struct [[gnu::packed]] DistributorRegisters {
    // Enables the forwarding of pending interrupts from the Distributor to the CPU interfaces.
    uint32_t distributor_control;

    // Provides information about the configuration of the GIC.
    uint32_t interrupt_controller;

    // Provides information about the implementer and revision of the Distributor.
    uint32_t distributor_implementer_identification;

    uint32_t _reserved1[5];
    uint32_t _implementation_defined[8];
    uint32_t _reserved2[16];

    // Each bit controls whether the corresponding interrupt is in Group 0 or Group 1.
    uint32_t interrupt_group[32];

    // Writing 1 to a Set-enable bit enables forwarding of the corresponding interrupt
    // from the Distributor to the CPU interfaces.
    uint32_t interrupt_set_enable[32];

    // Writing 1 to a Clear-enable bit disables forwarding of the corresponding interrupt
    // from the Distributor to the CPU interfaces.
    uint32_t interrupt_clear_enable[32];

    // Writing 1 to a Set-pending bit sets the status of the corresponding peripheral
    // interrupt to pending.
    uint32_t interrupt_set_pending[32];

    // Writing 1 to a Clear-pending bit clears the pending state of the corresponding
    // peripheral interrupt.
    uint32_t interrupt_clear_pending[32];

    // Writing to a Set-active bit Activates the corresponding interrupt.
    // These registers are used when preserving and restoring GIC state.
    uint32_t interrupt_set_active[32];

    // Writing to a Clear-active bit Deactivates the corresponding interrupt.
    // These registers are used when preserving and restoring GIC state.
    uint32_t interrupt_clear_active[32];

    // Provide an 8-bit priority field for each interrupt supported by the GIC.
    // This field stores the priority of the corresponding interrupt.
    uint32_t interrupt_priority[255];

    uint32_t _reserved3;

    // Provide an 8-bit CPU targets field for each interrupt supported by the GIC.
    // This field stores the list of target processors for the interrupt.
    const uint32_t interrupt_processor_targets_ro[8];
    uint32_t interrupt_processor_targets[247];

    uint32_t _reserved4;

    // Provide a 2-bit Int_config field for each interrupt supported by the GIC.
    // This field identifies whether the corresponding interrupt is edge-triggered or level-sensitive.
    uint32_t interrupt_configuration_registers[64];
};

struct [[gnu::packed]] CPUInterfaceRegisters {
    // Enables the signaling of interrupts by the CPU interface to the connected processor,
    // and provides additional top-level control of the CPU interface.
    // In a GICv2 implementation, this includes control of the end of interrupt (EOI) behavior.
    uint32_t cpu_interface_control;

    // Provides an interrupt priority filter. Only interrupts with higher priority
    // than the value in this register are signaled to the processor.
    uint32_t interrupt_priority_mask;

    // Defines the point at which the priority value fields split into two parts,
    // the group priority field and the subpriority field.
    uint32_t binary_point;

    // The processor reads this register to obtain the interrupt ID of the signaled interrupt.
    const uint32_t interrupt_acknowledge;

    // A processor writes to this register to inform the CPU interface
    // that it has completed the processing of the specified interrupt.
    uint32_t end_of_interrupt;

    // Indicates the Running priority of the CPU interface.
    const uint32_t running_priority;

    // Indicates the Interrupt ID, and processor ID if appropriate,
    // of the highest priority pending interrupt on the CPU interface.
    const uint32_t highest_priority_pending_interrupt;

    // A Binary Point Register for handling Group 1 interrupts.
    uint32_t aliased_binary_point;

    // An Interrupt Acknowledge register for handling Group 1 interrupts.
    const uint32_t aliased_interrupt_acknowledge_register;

    // An end of interrupt register for handling Group 1 interrupts.
    uint32_t aliased_end_of_interrupt;

    // Provides a Highest Priority Pending Interrupt register for the handling of Group 1 interrupts.
    const uint32_t aliased_highest_priority_pending_interrupt;
};

}