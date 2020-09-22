#ifndef MISTIX_KERNEL_DRIVERS_ATA_H
#define MISTIX_KERNEL_DRIVERS_ATA_H

#define WORDS_PER_SECTOR 256
#define BYTES_PER_SECTOR 512

#include "../types.h"

typedef struct {
    uint16_t data_port;
    uint16_t error_port;
    uint16_t sector_count_port;
    uint16_t lba_low_port;
    uint16_t lba_mid_port;
    uint16_t lba_high_port;
    uint16_t device_port;
    uint16_t command_port;
    uint16_t control_port;

    uint32_t capacity; // in sectors

    bool master;
} ata_t;

void ata_identify(ata_t* ata);
void ata_flush(ata_t* ata);
void ata_read28(ata_t* ata, uint32_t lba, uint8_t count, uint8_t* addr);
void ata_write28(ata_t* ata, uint32_t lba, uint8_t count, uint8_t* addr);
void ata_init(ata_t* ata, uint16_t port_base, bool master);

#endif // MISTIX_KERNEL_DRIVERS_ATA_H