#pragma once

#include <wisterialib/common.hpp>

void malloc_init();

void* malloc(size_t);

//malloc, but 4 bytes aligned
void* malloc_4(size_t);

void free(void*);

#ifdef DEBUG
void malloc_dump();
#endif
