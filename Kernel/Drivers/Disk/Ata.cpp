#include "Ata.hpp"

#include <Hardware/Port.hpp>
#include <Libkernel/Graphics/VgaTUI.hpp>

#include <Wisterialib/Common.hpp>

// comand port commands
#define IDENTIFY_COMMAND 0xEC
#define WRITE_SECTORS_COMMAND 0x30
#define READ_SECTORS_COMMAND 0x20
#define FLUSH_COMMAND 0xE7

// status register bits
#define ERR 0 // Indicates an error occurred
#define IDX 1 // Index. Always set to zero
#define CORR 2 // Corrected data. Always set to zero
#define DRQ 3 // Set when the drive has PIO data to transfer
#define SRV 4 // Overlapped Mode Service Request
#define DF 5 // Drive Fault Error
#define RDY 6 // Bit is clear when drive is spun down, or after an error. Set otherwise
#define BSY 7 // Indicates the drive is preparing to send/receive data

// error register bits
#define AMND 0 // Address mark not found
#define TKZNF 1 // Track zero not found
#define ABRT 2 // Aborted command
#define MCR 3 // Media change request
#define IDNF 4 // ID not found
#define MC 5 // Media changed
#define UNC 6 // Uncorrectable data error
#define BBK 7 // Bad Block detected

// macro, which gives the drive a 400ns delay to reset
// DRQ and set BSY bits
#define make_400ns_delay()            \
    {                                 \
        for (int i = 0; i < 5; i++) { \
            inb(m_command_port);      \
        }                             \
    }

// macro, which waits until device doen't have an apropiate value,
// also checks an error bit and processes it, if an error occurs
#define wait_bit(bit, value)                                                     \
    {                                                                            \
        uint8_t status = inb(m_command_port);                                    \
        while (((status >> bit) & 1) != (value & 1) && !((status >> ERR) & 1)) { \
            status = inb(m_command_port);                                        \
        }                                                                        \
                                                                                 \
        if (unlikely((status >> ERR) & 1)) {                                     \
            VgaTUI::Print("Recived error from the drive: \n");                      \
            handle_error(inb(m_error_port));                                     \
            return false;                                                        \
        }                                                                        \
    }

namespace Kernel::Drivers {

Ata::Ata(uint16_t port_base, bool master, DriverEntity driver_entity)

    : DiskDriver(driver_entity)
    , m_data_port(port_base)
    , m_error_port(port_base + 0x1)
    , m_sector_count_port(port_base + 0x2)
    , m_lba_low_port(port_base + 0x3)
    , m_lba_mid_port(port_base + 0x4)
    , m_lba_high_port(port_base + 0x5)
    , m_device_port(port_base + 0x6)
    , m_command_port(port_base + 0x7)
    , m_control_port(port_base + 0x206)
    , m_master(master)
{
}

bool Ata::install()
{
    outb(m_device_port, m_master ? 0xA0 : 0xB0);

    outb(m_sector_count_port, 0x0);

    outb(m_lba_low_port, 0x0);
    outb(m_lba_mid_port, 0x0);
    outb(m_lba_high_port, 0x0);

    outb(m_command_port, IDENTIFY_COMMAND);

    uint8_t status = inb(m_command_port);

    if (status == 0) {
        VgaTUI::Print("The drive does not exist");
        return false;
    }

    wait_bit(BSY, false);

    if (inb(m_lba_mid_port) && inb(m_lba_high_port)) {
        VgaTUI::Print("The drive is not ATA");
        return false;
    }

    wait_bit(DRQ, true);

    for (size_t i = 0; i < 256; i++) {
        uint16_t data = inw(m_data_port);

        switch (i) {
        /*
        words 60 and 61 contains the total number
        of 28 bit LBA addressable sectors on the drive
        */
        case 60:
            m_capacity = data;
            break;
        case 61:
            m_capacity |= ((uint32_t)data << 16);
            break;
        default:
            break;
        }
    }

    return true;
}

bool Ata::flush()
{
    outb(m_command_port, FLUSH_COMMAND);

    make_400ns_delay();

    wait_bit(BSY, false);

    return true;
}

bool Ata::read28(uint32_t lba, uint8_t sector_count, uint8_t* addr)
{
    outb(m_device_port, (m_master ? 0xE0 : 0xF0) | ((lba >> 24) & 0x0F));
    outb(m_sector_count_port, sector_count);

    outb(m_lba_low_port, (lba & 0xFF));
    outb(m_lba_mid_port, ((lba >> 8) & 0xFF));
    outb(m_lba_high_port, ((lba >> 16) & 0xFF));

    outb(m_command_port, READ_SECTORS_COMMAND);

    make_400ns_delay();

    wait_bit(BSY, false);

    for (size_t i = 0; i < sector_count * WORDS_PER_SECTOR; i++) {
        uint16_t read_word = inw(m_data_port);
        addr[2 * i + 0] = (read_word >> 0) & 0xFF;
        addr[2 * i + 1] = (read_word >> 8) & 0xFF;
        wait_bit(BSY, false);
    }

    return true;
}

bool Ata::write28(uint32_t lba, uint8_t sector_count, uint8_t* addr)
{
    outb(m_device_port, (m_master ? 0xE0 : 0xF0) | ((lba >> 24) & 0x0F));
    outb(m_sector_count_port, sector_count);

    outb(m_lba_low_port, (lba & 0xFF));
    outb(m_lba_mid_port, ((lba >> 8) & 0xFF));
    outb(m_lba_high_port, ((lba >> 16) & 0xFF));

    outb(m_command_port, WRITE_SECTORS_COMMAND);

    make_400ns_delay();

    wait_bit(BSY, false);

    for (size_t i = 0; i < sector_count * WORDS_PER_SECTOR; i++) {
        outw(m_data_port, ((uint16_t)(addr[2 * i + 1]) << 8) | ((uint16_t)addr[2 * i + 0] << 0));
        flush();
    }

    return true;
}

bool Ata::read(uint32_t lba, uint8_t sector_count, void* addr)
{
    if (addressing_mode == AdressingMode::Mode28) {
        return read28(lba, sector_count, reinterpret_cast<uint8_t*>(addr));
    }

    return false; // unimplemetnted for now :(
}

bool Ata::write(uint32_t lba, uint8_t sector_count, void* addr)
{
    if (addressing_mode == AdressingMode::Mode28) {
        return write28(lba, sector_count, reinterpret_cast<uint8_t*>(addr));
    }

    return false; // unimplemetnted for now :(
}

inline void Ata::handle_error(uint8_t error)
{
    static char* error_to_description[] = {
        "Address mark not found\n",
        "Track zero not found\n",
        "Aborted command\n",
        "Media change request\n",
        "ID not found\n",
        "Media changed\n",
        "Uncorrectable data error\n",
        "Bad Block detected\n",
    };

    VgaTUI::Print(error_to_description[error]);
}

}