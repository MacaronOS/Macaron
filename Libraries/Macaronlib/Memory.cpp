#include <Common.hpp>
#include <Memory.hpp>

void* memset(void* ptr, int value, size_t num)
{
    for (size_t i = (size_t)ptr; i < (size_t)ptr + num; i++) {
        *((uint8_t*)i) = value;
    }
    return ptr;
}

void* memcpy(void* write, const void* read, size_t num)
{
    for (size_t i = 0; i < num; i++) {
        ((uint8_t*)write)[i] = ((uint8_t*)read)[i];
    }
    return write;
}

int strcmp(const char* a, const char* b)
{
    while (*a == *b && *a != 0 && *b != 0) {
        a++;
        b++;
    }

    if (*a < *b) {
        return -1;
    }
    if (*a > *b) {
        return 1;
    }
    return 0;
}

size_t strlen(const char* str)
{
    size_t i = 0;
    while (str[i])
        i++;
    return i;
}