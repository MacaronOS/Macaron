#pragma once

#include <Macaronlib/Common.hpp>

namespace Kernel::Devices {

struct PL050Registers {
    uint32_t control;
    const uint32_t status;
    uint32_t data;
    uint32_t clock_divisor;
    const uint32_t interrupt_identification;
};

// PL050 status register masks can be found here:
// https://developer.arm.com/documentation/ddi0143/c/programmer-s-model/register-descriptions/kmistat---7-----0x04-

constexpr auto pl050_receiver_busy = 1 << 3;
constexpr auto pl050_ready_to_receive = 1 << 4;
constexpr auto pl050_sender_busy = 1 << 5;
constexpr auto pl050_ready_to_send = 1 << 6;

// PL050 control register masks can be found here:
// https://developer.arm.com/documentation/ddi0143/c/programmer-s-model/register-descriptions/kmicr---6-----0x00-

constexpr auto pl050_enable = 1 << 2;
constexpr auto pl050_enable_receiver_interrupt = 1 << 4;

enum class PL050Status {
    Uninitialized,
    Initialized,
    InitializationFailed,
};

class PL050Controller {
public:
    PL050Status initialize(uintptr_t kmi_base);

    void send_command(uint8_t cmd);
    void send_command_and_data(uint8_t cmd, uint8_t data);
    uint8_t receive_data();
    void enable_with_interrupts();

private:
    void assure_data_sent();

private:
    PL050Status m_initialization_status { PL050Status::Uninitialized };
    volatile PL050Registers* m_registers {};
};

}