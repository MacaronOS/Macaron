#pragma once

#include "../Common.hpp"

#define O_CREAT (1 << 3)
#define O_TRUNC (1 << 6)
#define O_APPEND (1 << 7)
#define O_DIRECTORY (1 << 9)

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#define MAP_SHARED 0x01
#define MAP_PRIVATE 0x02
#define MAP_FIXED 0x10
#define MAP_ANONYMOUS 0x20
#define MAP_STACK 0x40
#define MAP_NORESERVE 0x80

#define PROT_READ 0x1
#define PROT_WRITE 0x2
#define PROT_EXEC 0x4
#define PROT_NONE 0x0

#define AF_LOCAL 1
#define AF_UNIX AF_LOCAL

#define SOCK_STREAM 1

#define BGA_SWAP_BUFFERS 1

#define CLOCK_REALTIME 0
#define CLOCK_MONOTONIC 1
#define CLOCK_REALTIME_COARSE 5
#define CLOCK_MONOTONIC_COARSE 6

struct rtc_time {
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;
    int tm_wday; /* unused */
    int tm_yday; /* unused */
    int tm_isdst; /* unused */
};

#define RTC_RD_TIME 0

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

    Kill = 37,

    Ioctl = 54,

    Sigaction = 67,

    Select = 82,

    Mmap = 90,

    Sigreturn = 119,

    Sigprocmask = 126,

    GetDents = 141,

    SchedYield = 158,

    ClockGettime = 265,

    Socket = 359,
    Bind = 361,
    Connect = 362,

    WriteString,

    CreateSharedBuffer,
    GetSharedBuffer,

    CanRead,

    PTSName,

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

struct [[gnu::packed]] linux_dirent {
    uint32_t d_ino; /* Inode number */
    uint32_t d_off; /* Offset to next linux_dirent */
    uint16_t d_reclen; /* Length of this linux_dirent */
    char d_name[]; /* Filename (null-terminated) */
};

typedef uint32_t sigset_t;

struct sigaction {
    void (*sa_handler)(int);
    sigset_t sa_mask;
    int sa_flags;
};