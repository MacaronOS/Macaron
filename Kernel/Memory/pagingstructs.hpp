#pragma once
#include <Wisterialib/Common.hpp>

namespace Kernel::Memory {

// https://wiki.osdev.org/Exceptions#Page_Fault
enum class PageFaultFlag {
    Present, // the page fault was caused by a page-protection violation. When not set, it was caused by a non-present page
    Write, // the page fault was caused by a write access. When not set, it was caused by a read access.
    User, // the page fault was caused while CPL = 3. This does not necessarily mean that the page fault was a privilege violation.
    ReservedWrite, // one or more page directory entries contain reserved bits which are set to 1. This only applies when the PSE or PAE flags in CR4 are set to 1.
    InstructionFetch, // the page fault was caused by an instruction fetch. This only applies when the No-Execute bit is supported and enabled
};

typedef union {
    struct {
        uint32_t present : 1;
        uint32_t rw : 1;
        uint32_t user_mode : 1;
        uint32_t reserved_1 : 2;
        uint32_t access : 1;
        uint32_t dirty : 1;
        uint32_t reserved_2 : 2;
        uint32_t available : 3;
        uint32_t frame_adress : 20;
    };
    uint32_t _bits;
} PTEntry;

typedef struct {
    PTEntry entries[1024];
} PageTable;

typedef union {
    struct {
        uint32_t present : 1;
        uint32_t rw : 1;
        uint32_t user_mode : 1;
        uint32_t write_through : 1;
        uint32_t cache : 1; // if set, page table will be cached
        uint32_t access : 1;
        uint32_t reserved : 1;
        uint32_t page_size : 1; // 0 - 4kb, 1 - 4mb
        uint32_t global_page : 1; // ignored
        uint32_t available : 3;
        uint32_t pt_base : 20;
    };
    uint32_t _bits;
} PDEntry;

typedef struct {
    PDEntry entries[1024];
} PageDir;

}