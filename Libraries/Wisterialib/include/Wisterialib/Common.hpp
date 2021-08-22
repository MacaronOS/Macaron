#pragma once

typedef unsigned long  uint64_t;
typedef unsigned int   uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char  uint8_t;

typedef long           int64_t;
typedef signed int     int32_t;
typedef signed short   int16_t;
typedef signed char    int8_t;
typedef uint32_t       size_t;

typedef decltype(nullptr) nullptr_t;

#define false   0
#define true    1

#define unlikely(expr)  __builtin_expect(!!(expr), 0)
#define likely(expr)    __builtin_expect(!!(expr), 1)

#define KB 1024

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))

extern void* malloc(size_t);
extern void free(void*);

inline void* operator new(unsigned long size)
{
    return malloc(size);
}

inline void operator delete(void* ptr)
{
    return free(ptr);
}

inline void operator delete(void* ptr, unsigned long)
{
    return free(ptr);
}

inline void* operator new[](unsigned long size)
{
    return malloc(size);
}

inline void operator delete[](void* ptr)
{
    return free(ptr);
}

inline void operator delete[](void* ptr, unsigned long)
{
    return free(ptr);
}

inline void* operator new(unsigned long, void* ptr)
{
    return ptr;
}

inline void* operator new[](unsigned long, void* ptr)
{
    return ptr;
}

