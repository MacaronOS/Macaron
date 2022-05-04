#pragma once

#include "Common.hpp"

#define BITMAP_NULL -1

class Bitmap {
public:
    Bitmap() = default;
    Bitmap(uintptr_t location, size_t size);
    Bitmap(size_t size);
    ~Bitmap();

    Bitmap(const Bitmap&);
    Bitmap& operator=(const Bitmap&);
    Bitmap(Bitmap&&);
    Bitmap& operator=(Bitmap&&);

    size_t size() const;
    size_t memory_size() const;

    bool operator[](size_t index);
    void set_true(size_t index);
    void set_false(size_t index);
    size_t find_first_zero();
    size_t occupy_sequential(size_t size);
    void fill();
    void clear();

    static Bitmap wrap(uintptr_t location, size_t size);

private:
    bool m_self_created {}; // indecates if a bitmap was created or wraped
    size_t m_size { 0 }; // the number of elements in the bitmap
    size_t m_memory_size { 0 }; // the size of the bitmap in memory
    uint32_t* m_array { nullptr };
};
