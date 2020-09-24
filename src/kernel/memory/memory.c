#include "memory.h"
#include "monitor.h"
#include "types.h"

void* memset(void* ptr, int value, size_t num)
{
    for (size_t i = (size_t)ptr; i < (size_t)ptr + num; i++) {
        *(uint8_t*)i = value;
    }
}

void* memcpy(uint8_t* write, uint8_t* read, size_t num)
{
    for (size_t i = 0; i < num; i++) {
        write[i] = read[i];
    }
}