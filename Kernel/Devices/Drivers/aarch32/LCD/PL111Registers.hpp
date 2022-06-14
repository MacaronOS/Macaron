#pragma once

#include <Macaronlib/Common.hpp>

namespace Kernel::Devices {

// PL111 registers description can be found here:
// https://developer.arm.com/documentation/ddi0293/c/programmer-s-model/register-descriptions

struct [[gnu::packed]] PL111Registers {
    uint32_t horizontal_axis_panel_control;
    uint32_t vertical_axis_panel_control;
    uint32_t clock_and_signal_polarity_control;
    uint32_t line_end_control;

    // These registers are the color LCD DMA base address registers.
    // They program the base address of the frame buffer.
    uint32_t upper_panel_frame_base_address;
    uint32_t lower_panel_frame_base_address;

    // This register controls the operating mode, and the panel pixel parameters.
    uint32_t control;

    uint32_t interrupt_mask_set_clear;
    const uint32_t raw_interrupt_status;
    const uint32_t masked_interrupt_status;

    // Writing a logic 1 to the relevant bit in this register
    // clears the corresponding interrupt.
    uint32_t interrupt_clear;

    // These registers contain an approximate value of the upper
    // and lower panel data DMA addresses when read.
    const uint32_t upper_panel_current_address_value;
    const uint32_t lower_panel_current_address_value;
};

// PL111 control register masks can be found here:
// https://developer.arm.com/documentation/ddi0293/c/programmer-s-model/register-descriptions/lcd-control-register

constexpr auto pl111_enable = 1 << 0;
constexpr auto pl111_24bpp = 5 << 1;
constexpr auto pl111_color_mode = 0 << 4;
constexpr auto pl111_tft = 1 << 5;
constexpr auto pl111_rgb = 0 << 8;
constexpr auto pl111_bgr = 1 << 8;
constexpr auto pl111_power_enable = 1 << 11;

}