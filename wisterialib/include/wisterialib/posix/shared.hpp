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
    
    Lseek = 19,
    GetPid = 20,

    Ioctl = 54,

    Select = 82,

    Mmap = 90,

    ClockGettime = 265,

    Socket = 359,
    Bind = 361,
    Connect = 362,

    WriteString,

    CreateSharedBuffer,
    GetSharedBuffer,

    CanRead,

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

constexpr uint32_t FD_SIZE = 32;

#define FD_ZERO(set) memset((set), 0, sizeof(fd_set))
#define FD_SET(set, fd) ((set)->bits[fd / 8] |= (1 << (fd) % 8))
#define FD_CLR(set, fd) ((set)->bits[fd / 8] &= ~(1 << (fd) % 8))
#define FD_IS_SET(set, fd) (((set)->bits[fd / 8] >> ((fd) % 8)) & 1)

struct fd_set {
    uint8_t bits[FD_SIZE / 8];
};

struct MousePacket {
    uint8_t left_btn : 1;
    uint8_t right_btn : 1;
    uint8_t middle_btn : 1;
    int16_t x_move;
    int16_t y_move;
    uint8_t align : 2; // so file descriptor offset is correct after overflow
};

struct timespec {
    size_t tv_sec;
    size_t tv_nsec;
};