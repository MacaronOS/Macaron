#include <Memory.hpp>
#include <Common.hpp>

void* memset(void* ptr, int value, size_t num)
{
    for (size_t i = (size_t)ptr; i < (size_t)ptr + num; i++) {
        *((uint8_t*)i) = value;
    }
    return ptr;
}

void* memcpy(void* write, void* read, size_t num)
{
    for (size_t i = 0; i < num; i++) {
        ((uint8_t*)write)[i] = ((uint8_t*)read)[i];
    }
    return write;
}