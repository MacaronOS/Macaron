#ifndef MISTIX_KERNEL_FS_EXT2_H
#define MISTIX_KERNEL_FS_EXT2_H

#include "../types.h"
#include "../drivers/ata.h"

#define EXT2_STATE_CLEAN     1
#define EXT2_STATE_ERRORS    2

#define EXT2_ERROR_HANDLING_IGNORE           1
#define EXT2_ERROR_HANDLING_REMOUNT_R_ONLY   2
#define EXT2_ERROR_HANDLING_KERNEL_PANIK     3

typedef struct {
    uint32_t inodes_count; // Total number of inodes in file system
    uint32_t blocks_count; // Total number of blocks in file system
    uint32_t superblock_blocks_count; // Number of blocks reserved for superuser
    uint32_t unallocated_blocks_count; // Total number of unallocated blocks
    uint32_t unallocated_inodes_count; //Total number of unallocated inodes
    uint32_t superblock_block_number; // Block number of the block containing the superblock
    uint32_t block_shift; // log2 (block size) - 10. (In other words, the number to shift 1,024 to the left by to obtain the block size)
    uint32_t fragment_shift; // log2 (fragment size) - 10. (In other words, the number to shift 1,024 to the left by to obtain the fragment size)
    uint32_t blocks_per_block_group; // Number of blocks in each block group
    uint32_t fragments_per_block_group; // Number of fragments in each block group
    uint32_t inodes_per_block_group; // Number of inodes in each block group
    uint32_t last_mount_time; // in POSIX time
    uint32_t last_written_time; // in POSIX time

    uint16_t mounted_since_consistency_check; // Number of times the volume has been mounted since its last consistency check
    uint16_t mounted_since_consistency_check_allowed; // Number of mounts allowed before a consistency check (fsck) must be done
    uint16_t magic; // Ext2 signature (0xef53)
    uint16_t file_system_state; // File system state
    uint16_t error_handling_method; // What to do when an error is detected
    uint16_t version_low; // low portion of the version

    uint32_t consistency_check_time; // Time of the last consistency check (in POSIX)
    uint32_t consistency_check_interval; // Interval between forced consistency checks
    uint32_t os_id; // Operating system ID from which the filesystem on this volume was created
    uint32_t version_high; // high portion of the version

    uint16_t reserved_blocks_user_id; // User ID that can use reserved blocks
    uint16_t reserved_blocks_group_id; // Group ID that can use reserved blocks 
} __attribute__((packed)) ext2_superblock_t;

void ext2_init(ata_t* ata);

#endif // MISTIX_KERNEL_FS_EXT2_H