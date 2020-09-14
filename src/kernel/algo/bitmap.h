#ifndef MISTIX_KERNEL_ALGO_BITMAP_H
#define MISTIX_KERNEL_ALGO_BITMAP_H

#include "types.h"

#define BITMAP_NULL -1

typedef struct {
    uint32_t size; // size of the bitset
    uint32_t location; // location of the bitset
} bitmap_t;

bitmap_t bitmap_init(size_t sz);
uint32_t bitmap_init_at_location(bitmap_t* bitmap, size_t sz, void* location);
void bitmap_set_true(bitmap_t* bitmap, size_t pos);
void bitmap_set_false(bitmap_t* bitmap, size_t pos);
bool bitmap_test(bitmap_t* bitmap, size_t pos);
uint32_t bitmap_find_first_zero(bitmap_t* bitmap);
void bitmap_set_all(bitmap_t* bitmap, bool val);
void bitmap_clear(bitmap_t* bitmap);

#endif // MISTIX_KERNEL_ALGO_BITMAP_H