#pragma once

#include "Common.hpp"

extern "C" {

void* memset(void* ptr, int value, size_t num);
void* memcpy(void* write, const void* read, size_t num);
int strcmp(const char* a, const char* b);
size_t strlen(const char* str);
}
