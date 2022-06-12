#include "PL050Controller.hpp"
#include <Tasking/MemoryDescription/MemoryDescription.hpp>
#include <Tasking/MemoryDescription/SharedVMArea.hpp>

namespace Kernel::Devices {

using namespace Tasking;

constexpr auto response_acknowledged = 0xfa;
constexpr auto response_resend = 0xfe;

PL050Status PL050Controller::initialize(uintptr_t kmi_base)
{
    if (m_initialization_status != PL050Status::Uninitialized) {
        return m_initialization_status;
    }

    auto pl050_area = kernel_memory_description.allocate_memory_area<SharedVMArea>(
        sizeof(PL050Registers),
        VM_READ | VM_WRITE,
        true);

    if (!pl050_area) {
        ASSERT_PANIC("[PL050Controller] Could not allocate sp804 vmarea");
    }

    auto pl050_registers_virtual = pl050_area.result()->vm_start();

    VMM::the().map_memory(
        pl050_registers_virtual,
        kmi_base,
        sizeof(PL050Registers),
        1);

    m_registers = reinterpret_cast<PL050Registers*>(pl050_registers_virtual);
    m_initialization_status = PL050Status::Initialized;
    return m_initialization_status;
}

void PL050Controller::send_command(uint8_t cmd)
{
    if (m_initialization_status != PL050Status::Initialized) {
        ASSERT_PANIC("[PL050Controller] Used before initialization");
    }

    while (!(m_registers->status & pl050_ready_to_send)) { }
    m_registers->data = cmd;
    assure_data_sent();
}

void PL050Controller::send_command_and_data(uint8_t cmd, uint8_t data)
{
    if (m_initialization_status != PL050Status::Initialized) {
        ASSERT_PANIC("[PL050Controller] Used before initialization");
    }

    // Sending command.
    while (!(m_registers->status & pl050_ready_to_send)) { }
    m_registers->data = cmd;
    assure_data_sent();

    // Sending data right after the command.
    m_registers->data = data;
    assure_data_sent();
}

void PL050Controller::enable_with_interrupts()
{
    if (m_initialization_status != PL050Status::Initialized) {
        ASSERT_PANIC("[PL050Controller] Used before initialization");
    }

    auto tmp = m_registers->control;
    tmp |= pl050_enable;
    tmp |= pl050_enable_receiver_interrupt;
    m_registers->control = tmp;
}

uint8_t PL050Controller::receive_data()
{
    if (m_initialization_status != PL050Status::Initialized) {
        ASSERT_PANIC("[PL050Controller] Used before initialization");
    }

    while (!(m_registers->status & pl050_ready_to_receive)) { }
    return m_registers->data;
}

void PL050Controller::assure_data_sent()
{
    int attempts = 5;
    for (; attempts > 0; attempts--) {
        while (m_registers->status & pl050_sender_busy) { }
        auto tmp = m_registers->data;
        if (tmp == response_acknowledged) {
            break;
        }
        if (tmp == response_resend) {
            continue;
        }
        ASSERT_PANIC("[PL050Controller] Error while sending");
    }

    if (attempts == 0) {
        ASSERT_PANIC("[PL050Controller] Could not send data");
    }
}

}