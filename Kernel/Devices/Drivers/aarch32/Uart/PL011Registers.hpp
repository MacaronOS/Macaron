#pragma once

#include <Macaronlib/Common.hpp>

namespace Kernel::Devices {

// PL011 registers description are taken from here:
// https://developer.arm.com/documentation/ddi0183/f/programmer-s-model/register-descriptions

struct [[gnu::packed]] PL011Registers {
    uint32_t data;

    // A write to this register clears the framing, parity, break, and overrun errors.
    uint32_t receive_status_error_clear;

    const uint32_t _reserved0[4];

    const uint32_t flags;

    const uint32_t _reserved1;

    uint32_t low_power_counter;

    // This register is the integer part of the baud rate divisor value.
    uint32_t integer_baud_rate;
    // This register is the fractional part of the baud rate divisor value.
    uint32_t fractional_baud_rate;

    uint32_t line_control;
    uint32_t control;
    uint32_t interrupt_fifo_level_select;
    uint32_t interrupt_mask_set_clear;

    // On a read this register gives the current raw status value of the corresponding interrupt.
    const uint32_t raw_interrupt_status;

    // On a read this register gives the current masked status value of the corresponding interrupt.
    const uint32_t masked_interrupt_status;

    // On a write of 1, the corresponding interrupt is cleared.
    uint32_t interrupt_clear;
};

// PL011 line control register masks are taken from here:
// https://developer.arm.com/documentation/ddi0183/f/programmer-s-model/register-descriptions/line-control-register--uartlcr-h

constexpr auto pl011_enable_fifo = 1 << 4;
constexpr auto pl011_word_length_8_bits = 3 << 5;

// PL011 control register masks are taken from here:
// https://developer.arm.com/documentation/ddi0183/f/programmer-s-model/register-descriptions/control-register--uartcr

constexpr auto pl011_uart_enable = 1 << 0;
constexpr auto pl011_transmit_enable = 1 << 8;
constexpr auto pl011_receive_enable = 1 << 9;

// PL011 interrupt mask set/clear register masks are taken from here:
// https://developer.arm.com/documentation/ddi0183/f/programmer-s-model/register-descriptions/interrupt-mask-set-clear-register--uartimsc

constexpr auto pl011_receive_interrupt_enable = 1 << 4;

// PL011 Flag register masks are taken from here:
// https://developer.arm.com/documentation/ddi0183/f/programmer-s-model/register-descriptions/flag-register--uartfr

constexpr auto pl011_uart_busy = 1 << 3;
constexpr auto pl011_receive_fifo_empty = 1 << 4;
constexpr auto pl011_transmit_fifo_full = 1 << 5;

}