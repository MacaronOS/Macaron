#pragma once

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

