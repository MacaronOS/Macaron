#ifndef MISTIX_KERNEL_TYPES_H
#define MISTIX_KERNEL_TYPES_H

typedef unsigned long  uint64_t;
typedef unsigned int   uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char  uint8_t;

typedef long           int64_t;
typedef signed int     int32_t;
typedef signed short   int16_t;
typedef signed char    int8_t;

typedef char           bool;
typedef int            size_t;

#define false 0
#define true 1

#define unlikely(expr)  __builtin_expect(!!(expr), 0)
#define likely(expr)    __builtin_expect(!!(expr), 1)

#endif // MISTIX_KERNEL_TYPES_H