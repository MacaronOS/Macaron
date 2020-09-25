#include "ata.hpp"

#include "../monitor.hpp"
#include "../port.hpp"
#include "../types.hpp"

// comand port commands
#define IDENTIFY_COMMAND        0xEC
#define WRITE_SECTORS_COMMAND   0x30
#define READ_SECTORS_COMMAND    0x20
#define FLUSH_COMMAND           0xE7

// status register bits
#define ERR     0 // Indicates an error occurred
#define IDX     1 // Index. Always set to zero
#define CORR    2 // Corrected data. Always set to zero
#define DRQ     3 // Set when the drive has PIO data to transfer
#define SRV     4 // Overlapped Mode Service Request
#define DF      5 // Drive Fault Error
#define RDY     6 // Bit is clear when drive is spun down, or after an error. Set otherwise
#define BSY     7 // Indicates the drive is preparing to send/receive data

// error register bits
#define AMND    0 // Address mark not found
#define TKZNF   1 // Track zero not found
#define ABRT    2 // Aborted command
#define MCR     3 // Media change request
#define IDNF    4 // ID not found
#define MC      5 // Media changed
#define UNC     6 // Uncorrectable data error
#define BBK     7 // Bad Block detected

static inline void ata_handle_error(uint8_t error)
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

    term_print(error_to_description[error]);
}

// macro, which gives the drive a 400ns delay to reset 
// DRQ and set BSY bits 
#define ata_400ns_delay(ata)          \
    {                                 \
        for (int i = 0; i < 5; i++) { \
            inb(ata->command_port);   \
        }                             \
    }

// macro, which waits until device doen't have an apropiate value,
// also checks an error bit and processes it, if an error occurs
#define ata_wait_bit(ata, bit, value)                                            \
    {                                                                            \
        uint8_t status = inb(ata->command_port);                                 \
        while (((status >> bit) & 1) != (value & 1) && !((status >> ERR) & 1)) { \
            status = inb(ata->command_port);                                     \
        }                                                                        \
                                                                                 \
        if (unlikely((status >> ERR) & 1)) {                                     \
            term_print("Recived error from the drive: \n");                      \
            ata_handle_error(inb(ata->error_port));                              \
            return;                                                              \
        }                                                                        \
    }

void ata_init(ata_t* ata, uint16_t port_base, bool master)
{
    ata->data_port = port_base;
    ata->error_port = port_base + 0x1;
    ata->sector_count_port = port_base + 0x2;
    ata->lba_low_port = port_base + 0x3;
    ata->lba_mid_port = port_base + 0x4;
    ata->lba_high_port = port_base + 0x5;
    ata->device_port = port_base + 0x6;
    ata->command_port = port_base + 0x7;
    ata->control_port = port_base + 0x206;

    ata->master = master;
}

void ata_identify(ata_t* ata)
{
    outb(ata->device_port, ata->master ? 0xA0 : 0xB0);

    outb(ata->sector_count_port, 0x0);

    outb(ata->lba_low_port, 0x0);
    outb(ata->lba_mid_port, 0x0);
    outb(ata->lba_high_port, 0x0);

    outb(ata->command_port, IDENTIFY_COMMAND);

    uint8_t status = inb(ata->command_port);

    if (status == 0) {
        term_print("The drive does not exist");
        return;
    }

    ata_wait_bit(ata, BSY, false);

    if (inb(ata->lba_mid_port) && inb(ata->lba_high_port)) {
        term_print("The drive is not ATA");
        return;
    }

    ata_wait_bit(ata, DRQ, true);

    for (size_t i = 0; i < 256; i++) {
        uint16_t data = inw(ata->data_port);

        switch (i) {
        /*
        words 60 and 61 contains the total number
        of 28 bit LBA addressable sectors on the drive
        */
        case 60:
            ata->capacity = data;
            break;
        case 61:
            ata->capacity |= ((uint32_t)data << 16);
            break;
        default:
            break;
        }
    }
}

void ata_read28(ata_t* ata, uint32_t lba, uint8_t count, void* addr)
{
    outb(ata->device_port, (ata->master ? 0xE0 : 0xF0) | ((lba >> 24) & 0x0F));
    outb(ata->sector_count_port, count);

    outb(ata->lba_low_port, (lba & 0xFF));
    outb(ata->lba_mid_port, ((lba >> 8) & 0xFF));
    outb(ata->lba_high_port, ((lba >> 16) & 0xFF));

    outb(ata->command_port, READ_SECTORS_COMMAND);

    ata_400ns_delay(ata);

    ata_wait_bit(ata, BSY, false);

    for (size_t i = 0; i < count * WORDS_PER_SECTOR; i++) {
        uint16_t read_word = inw(ata->data_port);
        ((uint8_t*)addr)[2 * i + 0] = (read_word >> 0) & 0xFF;
        ((uint8_t*)addr)[2 * i + 1] = (read_word >> 8) & 0xFF;
        ata_wait_bit(ata, BSY, false);
    }
}

void ata_flush(ata_t* ata)
{
    outb(ata->command_port, FLUSH_COMMAND);

    ata_400ns_delay(ata);
    
    ata_wait_bit(ata, BSY, false);
}

void ata_write28(ata_t* ata, uint32_t lba, uint8_t count, uint8_t* addr)
{
    outb(ata->device_port, (ata->master ? 0xE0 : 0xF0) | ((lba >> 24) & 0x0F));
    outb(ata->sector_count_port, count);

    outb(ata->lba_low_port, (lba & 0xFF));
    outb(ata->lba_mid_port, ((lba >> 8) & 0xFF));
    outb(ata->lba_high_port, ((lba >> 16) & 0xFF));

    outb(ata->command_port, WRITE_SECTORS_COMMAND);

    ata_400ns_delay(ata);

    ata_wait_bit(ata, BSY, false);

    for (size_t i = 0; i < count * WORDS_PER_SECTOR; i++) {
        outw(ata->data_port, ((uint16_t)(addr[2 * i + 1]) << 8) | ((uint16_t)addr[2 * i + 0] << 0));
        ata_flush(ata);
    }
}