#include "ext2.h"

#include "../drivers/ata.h"

#include "../monitor.h"

#define SUPERBLOCK_LOCATION 1024
#define SUPERBLOCK_SIZE     1024

ext2_superblock_t superblock;

void ext2_init(ata_t* ata) 
{
    ata_read28(ata, SUPERBLOCK_LOCATION / BYTES_PER_SECTOR, SUPERBLOCK_SIZE / BYTES_PER_SECTOR, &superblock);
    term_printd(superblock.magic);
}