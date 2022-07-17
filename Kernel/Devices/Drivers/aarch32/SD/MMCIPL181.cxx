#include "MMCIPL181.hpp"
#include <Tasking/MemoryDescription/MemoryDescription.hpp>
#include <Tasking/MemoryDescription/SharedVMArea.hpp>

namespace Kernel::Devices {

using namespace Tasking;

MMCIPL181 mmci_pl181;

constexpr uintptr_t MMCI_PL181_PHYSICAL_ADDRESS = 0x1c050000;

// Commands.
constexpr uint32_t MMCI_CMD_RESPONSE = 1 << 6;
constexpr uint32_t MMCI_CMD_LONG_RESP = 1 << 7;
constexpr uint32_t MMCI_CMD_INTERRUPT = 1 << 8;
constexpr uint32_t MMCI_CMD_PENDING = 1 << 9;
constexpr uint32_t MMCI_CMD_ENABLE = 1 << 10;

constexpr uint32_t MMCI_CMD_GO_IDLE_STATE = 0;
constexpr uint32_t MMCI_CMD_ALL_SEND_CID = 2;
constexpr uint32_t MMCI_CMD_SET_RELATIVE_ADDR = 3;
constexpr uint32_t MMCI_CMD_SELECT_CARD = 7;
constexpr uint32_t MMCI_CMD_IF_COND = 8;
constexpr uint32_t MMCI_CMD_SET_BLOCKSIZE = 16;
constexpr uint32_t MMCI_CMD_READ_SINGLE_BLOCK = 17;
constexpr uint32_t MMCI_CMD_SD_SEND_OP_COND = 41;
constexpr uint32_t MMCI_CMD_APP_CMD = 55;

constexpr uint32_t MMCI_SD_ARG_VDD_MASK = 0xff8000;
constexpr uint32_t MMCI_SD_OCR_CCS = 1 << 30;
constexpr uint32_t MMCI_SD_CRR_BUSY = 1 << 31;

// Status.
constexpr uint32_t MMCI_STAT_CRC_FAIL = 1 << 0;
constexpr uint32_t MMCI_STAT_DATA_CRC_FAIL = 1 << 1;
constexpr uint32_t MMCI_STAT_TIME_OUT = 1 << 2;
constexpr uint32_t MMCI_STAT_DATA_TIME_OUT = 1 << 3;
constexpr uint32_t MMCI_STAT_RX_OVERRUN = 1 << 5;
constexpr uint32_t MMCI_STAT_RESP_END = 1 << 6;
constexpr uint32_t MMCI_STAT_SENT = 1 << 7;
constexpr uint32_t MMCI_STAT_DATA_AVLBL = 1 << 21;

void MMCIPL181::install()
{
    auto pl181_area = kernel_memory_description.allocate_memory_area<SharedVMArea>(
        sizeof(PL181Registers),
        VM_READ | VM_WRITE,
        true);

    if (!pl181_area) {
        ASSERT_PANIC("[MMCIPL181] Could not allocate pl181 vmarea");
    }

    auto pl181_registers_virtual = pl181_area.result()->vm_start();

    VMM::the().map_memory(
        pl181_registers_virtual,
        MMCI_PL181_PHYSICAL_ADDRESS,
        sizeof(PL181Registers),
        1);

    m_registers = reinterpret_cast<volatile PL181Registers*>(pl181_registers_virtual);

    m_registers->clock_control = 0x1C6;
    m_registers->power_control = 0x86;

    send_command(MMCI_CMD_GO_IDLE_STATE, 0);

    // Voltage check.
    uint32_t vlt;
    send_command(MMCI_CMD_IF_COND | MMCI_CMD_RESPONSE, 0x1aa, &vlt);
    if (vlt & 0xfff != 0x1aa) {
        ASSERT_PANIC("[MMCIPL181] Voltage check failed");
        return;
    }

    // Generate a new relative card address.
    uint32_t resp;
    do {
        send_command(MMCI_CMD_APP_CMD | MMCI_CMD_RESPONSE, 0);
        send_command(MMCI_CMD_SD_SEND_OP_COND | MMCI_CMD_RESPONSE, MMCI_SD_OCR_CCS | MMCI_SD_ARG_VDD_MASK, &resp);
    } while (!(resp & MMCI_SD_CRR_BUSY));

    send_command(MMCI_CMD_ALL_SEND_CID | MMCI_CMD_RESPONSE | MMCI_CMD_LONG_RESP, 0);
    uint32_t rca;
    send_command(MMCI_CMD_SET_RELATIVE_ADDR | MMCI_CMD_RESPONSE, 0, &rca);
    rca &= 0xffff0000;

    // Register SD0.
    register_device<SDCardPL181>(0, rca, *this);
}

bool MMCIPL181::set_block_size(uint32_t rca, size_t block_size)
{
    select_card(rca);
    uint32_t res;
    send_command(MMCI_CMD_SET_BLOCKSIZE | MMCI_CMD_RESPONSE, block_size, &res);
    return res == 0x900;
}

bool MMCIPL181::read_block(uint32_t rca, size_t block, size_t block_size, uint32_t* buffer)
{
    select_card(rca);
    m_registers->data_length = block_size;
    m_registers->data_control = 0b11;

    send_command(MMCI_CMD_READ_SINGLE_BLOCK | MMCI_CMD_RESPONSE, block * block_size);

    size_t bytes_left = block_size;
    constexpr uint32_t error_flags = MMCI_STAT_DATA_CRC_FAIL | MMCI_STAT_DATA_TIME_OUT | MMCI_STAT_RX_OVERRUN;
    while (bytes_left && !(m_registers->status & error_flags) && m_registers->status & MMCI_STAT_DATA_AVLBL) {
        *buffer = m_registers->data_fifo[0];
        buffer++;
        bytes_left -= sizeof(uint32_t);
    }

    return bytes_left == 0;
}

void MMCIPL181::select_card(uint32_t rca)
{
    send_command(MMCI_CMD_SELECT_CARD | MMCI_CMD_RESPONSE, rca);
}

int MMCIPL181::send_command(uint32_t cmd, uint32_t arg, uint32_t* res)
{
    // Clear flags.
    m_registers->clear = 0x5ff;

    // Send command.
    m_registers->argument = arg;
    m_registers->command = cmd | MMCI_CMD_ENABLE;

    // Wait for execution.
    const uint32_t check_flags = cmd & MMCI_CMD_RESPONSE
        ? MMCI_STAT_RESP_END | MMCI_STAT_TIME_OUT | MMCI_STAT_CRC_FAIL
        : MMCI_STAT_SENT | MMCI_STAT_TIME_OUT;

    while (!(m_registers->status & check_flags)) { }

    // Check errors.
    constexpr uint32_t error_flags = MMCI_STAT_TIME_OUT | MMCI_STAT_CRC_FAIL;
    if (m_registers->status & error_flags) {
        return -1;
    }

    // Extract response.
    if (cmd & MMCI_CMD_RESPONSE && res) {
        if (cmd & MMCI_CMD_LONG_RESP) {
            res[0] = m_registers->response[0];
            res[1] = m_registers->response[1];
            res[2] = m_registers->response[2];
            res[3] = m_registers->response[3];
        } else {
            *res = m_registers->response[0];
        }
    }
    return 0;
}

}