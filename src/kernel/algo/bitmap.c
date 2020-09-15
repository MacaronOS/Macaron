#include "bitmap.h"
#include "../memory/kmalloc.h"
#include "monitor.h"
#include "types.h"

#define BITMAP_CHUNK_SIZE 32
#define BITMAP_CHUNK_COUNT(sz) (sz / BITMAP_CHUNK_SIZE + ((sz % BITMAP_CHUNK_SIZE > 0) ? 1 : 0))

bitmap_t bitmap_init(size_t sz)
{
    uint32_t location = kmalloc(BITMAP_CHUNK_COUNT(sz) * 4);

    bitmap_t bitmap;
    bitmap.size = sz;
    bitmap.location = location;

    bitmap_clear(&bitmap);

    return bitmap;
}
/*
put bitmap array at location, returns the size of array
*/
uint32_t bitmap_init_at_location(bitmap_t* bitmap, size_t sz, void* location)
{
    bitmap->size = sz;
    bitmap->location = location;

    return BITMAP_CHUNK_COUNT(bitmap->location) * BITMAP_CHUNK_SIZE;
}

void bitmap_set_true(bitmap_t* bitmap, size_t pos)
{
    *((uint32_t*)(bitmap->location + pos / BITMAP_CHUNK_SIZE * 4)) |= (1 << (BITMAP_CHUNK_SIZE - 1 - pos % BITMAP_CHUNK_SIZE));
}

void bitmap_set_false(bitmap_t* bitmap, size_t pos)
{
    *((uint32_t*)(bitmap->location + pos / BITMAP_CHUNK_SIZE * 4)) &= ~(1 << (BITMAP_CHUNK_SIZE - 1 - pos % BITMAP_CHUNK_SIZE));
}

bool bitmap_test(bitmap_t* bitmap, size_t pos)
{
    return ((*((uint32_t*)(bitmap->location + pos / BITMAP_CHUNK_SIZE * 4))) >> (BITMAP_CHUNK_SIZE - 1 - pos % BITMAP_CHUNK_SIZE)) & 1;
}

void bitmap_clear(bitmap_t* bitmap)
{
    for (uint32_t chunk = 0; chunk < BITMAP_CHUNK_COUNT(bitmap->size); chunk++) {
        ((uint32_t*)bitmap->location)[chunk] = 0;
    }
}

void bitmap_set_all(bitmap_t* bitmap, bool val) {
    for (uint32_t chunk = 0; chunk < BITMAP_CHUNK_COUNT(bitmap->size); chunk++) {
        ((uint32_t*)bitmap->location)[chunk] = (val & 1);
    }
}

uint32_t bitmap_find_first_zero(bitmap_t* bitmap)
{
    for (uint32_t chunk = 0; chunk < BITMAP_CHUNK_COUNT(bitmap->size); chunk++) {
        if (chunk != 0xFFFFFFFF) {
            for (uint32_t pos = 0; pos < 32; pos++) {
                if (((((uint32_t)((uint32_t*)bitmap->location)[chunk]) >> (BITMAP_CHUNK_SIZE - 1 - pos)) & 1) == 0) {
                    return chunk * BITMAP_CHUNK_SIZE + pos;
                }
            }
        }
    }

    return BITMAP_NULL;
}