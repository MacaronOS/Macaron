#pragma once

#include <Macaronlib/Common.hpp>

namespace Kernel::Devices {

// SP804 registers description can be found here:
// https://developer.arm.com/documentation/ddi0271/d/programmer-s-model/register-descriptions?lang=en

struct SP804Registers {
    // This register contains the value from which the counter is to decrement.
    // This is the value used to reload the counter when Periodic mode is enabled,
    // and the current count reaches zero.
    uint32_t load;

    // This register read-only register that gives the current value
    // of the decrementing counter.
    const uint32_t current_value;

    // This register enables the timer, sets a mode,
    // enables interrupts, sets prescale, selects 16/32 bit counter operation
    // and selects one-shot or wrapping counter mode.
    uint32_t control;

    // Any write to this register, clears the interrupt output from the counter.
    uint32_t interrupt_clear;

    // This register indicates the raw interrupt status from the counter.
    const uint32_t raw_interrupt_status;

    // This register indicates the masked interrupt status from the counter.
    const uint32_t masked_interrupt_status;

    // This register provides an alternative method of accessing the load Register.
    // The difference is that writes to TimerXBGLoad do not cause the counter to
    // restart from the new value immediately.
    uint32_t background_load;

    // The read-only register provide the peripheral options for the timer.
    const uint32_t peripheral_identification;

    // The read-only register used as a standard cross-peripheral identification system.
    const uint32_t PrimeCell_identification;
};

// SP804 control register masks can be found here:
// https://developer.arm.com/documentation/ddi0271/d/programmer-s-model/register-descriptions/control-register--timerxcontrol?lang=en

constexpr auto sp804_wrapping_mode = 0 << 0;
constexpr auto sp804_timer_size_32bit = 1 << 1;
constexpr auto sp804_timer_prescale_1 = (0 << 2 | 0 << 3);
constexpr auto sp804_interrupt_enable = 1 << 5;
constexpr auto sp804_timer_mode_periodic = 1 << 6;
constexpr auto sp804_timer_enable = 1 << 7;

}