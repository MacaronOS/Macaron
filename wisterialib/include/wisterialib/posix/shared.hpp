#pragma once
#include "../common.hpp"

enum class Syscall {
    Putc = 0,
    Exit,
    Fork,
    Read,
    Write,
    Open,
    Close,

    Execve = 9,

    Printd,

    Mmap = 90,

    WriteString,

    CreateSharedBuffer,
    GetSharedBuffer,

    END,
};

struct [[gnu::packed]] MmapParams {
    uint32_t start;
    uint32_t size;
    int prot;
    int flags;
    int fd;
    uint32_t offset;
};

struct CreateBufferResult {
    uint32_t id {};
    uint32_t mem {};
};