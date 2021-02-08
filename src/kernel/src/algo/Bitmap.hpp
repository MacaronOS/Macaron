#pragma once

#include "../types.hpp"

#define BITMAP_NULL -1

namespace kernel {

class Bitmap {
public:
    Bitmap() = default;
    Bitmap(uint32_t location, size_t size);
    Bitmap(size_t size);

    ~Bitmap();

    static Bitmap wrap(uint32_t location, size_t size);

    size_t size() const;
    size_t memory_size() const;

    bool operator[](const size_t index);
    void set_true(const size_t index);
    void set_false(const size_t index);
    size_t find_first_zero();
    void fill();
    void clear();

private:
    bool m_self_created {}; // indecates if a bitmap was created or wraped
    size_t m_size { 0 }; // the number of elements in the bitmap
    size_t m_memory_size { 0 }; // the size of the bitmap in memory
    uint32_t* m_array { nullptr };
};

}
