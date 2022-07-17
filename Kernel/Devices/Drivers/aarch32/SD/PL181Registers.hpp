#pragma once

#include <Macaronlib/Common.hpp>

namespace Kernel::Devices {

struct PL181Registers {
    // This register controls an external power supply.
    // It can switch the power on and off, and adjust the output voltage.
    uint32_t power_control;

    // This register enables PrimeCell MCI bus clock.
    uint32_t clock_control;

    // This register contains a 32-bit command argument,
    // which is sent to a card as part of a command message.
    uint32_t argument;

    // This register contains the command index and command type bits.
    uint32_t command;

    // This register contains the command index field of
    // the last command response received.
    const uint32_t command_response;

    // Thsese registers registers contain the status of a card,
    // which is part of the received response.
    const uint32_t response[4];

    // This register contains the data timeout period,
    // in card bus clock periods.
    uint32_t data_timer;

    // This register contains the number of data bytes to be transferred.
    // The value is loaded into the data counter when data transfer starts.
    uint32_t data_length;

    // This register sets data transfer direction, data transfer mode, dma
    // and data transfer.
    uint32_t data_control;

    // This register loads the value from the data_length register,
    // as data is transferred, the counter decrements the value until it reaches 0.
    const uint32_t data_count;

    // This register contains two types of flags:
    // Static - these remain asserted until they are cleared by writing to the Clear register.
    // Dynamic - these change state depending on the state of the underlying logic.
    const uint32_t status;

    // This register is a write-only register. The corresponding static status flags
    // can be cleared by writing a 1 to the corresponding bit in the register.
    uint32_t clear;

    // There are two interrupt mask registers, one for each interrupt request signal.
    uint32_t interrupt_mask[2];

    uint32_t _reserved1;

    // This register contains the remaining number of words to be
    // written to or read from the FIFO.
    const uint32_t fifo_counter;

    const uint32_t _reserved2[13];

    // The receive and transmit FIFOs can be read or written as 32-bit wide registers.
    // The FIFOs contain 16 entries on 16 sequential addresses.
    uint32_t data_fifo[16];
};

}