#pragma once

#include "types.hpp"

#define BITMAP_NULL -1

namespace kernel::algorithms {

class Bitmap {
private:
    size_t m_size { 0 };
    uint32_t* m_array { nullptr };

public:
    Bitmap() = default;
    Bitmap(uint32_t location, size_t size);
    Bitmap(size_t size);
    ~Bitmap();

    size_t size();

    bool operator[](const size_t index);
    void set_true(const size_t index);
    void set_false(const size_t index);
    size_t find_first_zero();
    void fill();
    void clear();
};

}
