#include "ATA.hpp"
#include <Hardware/x86/Port.hpp>
#include <Libkernel/Logger.hpp>

#define WORDS_PER_SECTOR 256
#define BYTES_PER_SECTOR 512

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

namespace Kernel::Devices {

ATA ata(0x1F0, true);

inline void ata_handle_error(uint8_t error)
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

    Log() << error_to_description[error] << "\n";
}

static inline uint32_t ata_minor(uint16_t port_base, bool master)
{
    return ((port_base == 0x1F0) ? 0 : 2) + !master;
}

ATA::ATA(uint16_t port_base, bool master)
    : BlockDeviceDriver<ATA>(3, ata_minor(port_base, master))
    , m_data_port(port_base)
    , m_error_port(port_base + 0x1)
    , m_sector_count_port(port_base + 0x2)
    , m_lba_low_port(port_base + 0x3)
    , m_lba_mid_port(port_base + 0x4)
    , m_lba_high_port(port_base + 0x5)
    , m_device_port(port_base + 0x6)
    , m_command_port(port_base + 0x7)
    , m_control_port(port_base + 0x206)
    , m_master(master) {
        ACKNOWLEDGE_DRIVER
    }

    size_t ATA::block_size()
{
    return BYTES_PER_SECTOR;
}

bool ATA::read_blocks(size_t block, size_t block_count, void* buffer)
{
    auto buf = reinterpret_cast<uint8_t*>(buffer);

    if (m_addressing_mode == AtaAdressingMode::Mode28) {
        while (block_count) {
            uint8_t sectors = min(block_count, 64);
            if (!read28(block, sectors, buf)) {
                return false;
            }
            block_count -= sectors;
            buf += sectors * BYTES_PER_SECTOR;
        }
    }

    return false;
}

bool ATA::write_blocks(size_t block, size_t block_count, void* buffer)
{
    auto buf = reinterpret_cast<uint8_t*>(buffer);

    if (m_addressing_mode == AtaAdressingMode::Mode28) {
        while (block_count) {
            uint8_t sectors = min(block_count, 64);
            if (!write28(block, sectors, buf)) {
                return false;
            }
            block_count -= sectors;
            buf += sectors * BYTES_PER_SECTOR;
        }
    }

    return false;
}

DriverInstallationResult ATA::try_install()
{
    outb(m_device_port, m_master ? 0xA0 : 0xB0);

    outb(m_sector_count_port, 0x0);

    outb(m_lba_low_port, 0x0);
    outb(m_lba_mid_port, 0x0);
    outb(m_lba_high_port, 0x0);

    outb(m_command_port, IDENTIFY_COMMAND);

    uint8_t status = inb(m_command_port);

    if (status == 0) {
        // The drive does not exist
        return DriverInstallationResult::Failed;
    }

    if (!wait_bit(BSY, false)) {
        return DriverInstallationResult::Failed;
    }

    if (inb(m_lba_mid_port) && inb(m_lba_high_port)) {
        // The drive is not ATA
        return DriverInstallationResult::Failed;
    }

    if (!wait_bit(DRQ, true)) {
        return DriverInstallationResult::Failed;
    }

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

    return DriverInstallationResult::Succeeded;
}

bool ATA::read28(uint32_t lba, uint8_t sector_count, uint8_t* addr)
{
    outb(m_device_port, (m_master ? 0xE0 : 0xF0) | ((lba >> 24) & 0x0F));
    outb(m_sector_count_port, sector_count);

    outb(m_lba_low_port, (lba & 0xFF));
    outb(m_lba_mid_port, ((lba >> 8) & 0xFF));
    outb(m_lba_high_port, ((lba >> 16) & 0xFF));

    outb(m_command_port, READ_SECTORS_COMMAND);

    make_400ns_delay();

    if (!wait_bit(BSY, false)) {
        return false;
    }

    for (size_t i = 0; i < sector_count * WORDS_PER_SECTOR; i++) {
        uint16_t read_word = inw(m_data_port);
        addr[2 * i + 0] = (read_word >> 0) & 0xFF;
        addr[2 * i + 1] = (read_word >> 8) & 0xFF;
        if (!wait_bit(BSY, false)) {
            return false;
        }
    }

    return true;
}

bool ATA::write28(uint32_t lba, uint8_t sector_count, uint8_t* addr)
{
    outb(m_device_port, (m_master ? 0xE0 : 0xF0) | ((lba >> 24) & 0x0F));
    outb(m_sector_count_port, sector_count);

    outb(m_lba_low_port, (lba & 0xFF));
    outb(m_lba_mid_port, ((lba >> 8) & 0xFF));
    outb(m_lba_high_port, ((lba >> 16) & 0xFF));

    outb(m_command_port, WRITE_SECTORS_COMMAND);

    make_400ns_delay();

    if (!wait_bit(BSY, false)) {
        return false;
    }

    for (size_t i = 0; i < sector_count * WORDS_PER_SECTOR; i++) {
        outw(m_data_port, ((uint16_t)(addr[2 * i + 1]) << 8) | ((uint16_t)addr[2 * i + 0] << 0));
        flush();
    }

    return true;
}

void ATA::make_400ns_delay()
{
    for (int i = 0; i < 5; i++) {
        inb(m_command_port);
    }
}

bool ATA::wait_bit(uint8_t bit, uint8_t val)
{
    uint8_t status = inb(m_command_port);
    while (((status >> bit) & 1) != (val & 1) && !((status >> ERR) & 1)) {
        status = inb(m_command_port);
    }

    if (unlikely((status >> ERR) & 1)) {
        Log() << "Recived error from the drive: \n";
        ata_handle_error(inb(m_error_port));
        return false;
    }

    return true;
}

bool ATA::flush()
{
    outb(m_command_port, FLUSH_COMMAND);
    make_400ns_delay();
    if (!wait_bit(BSY, false)) {
        return false;
    }
    return true;
}

}