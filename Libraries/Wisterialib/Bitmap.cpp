#include <Bitmap.hpp>
#include <common.hpp>

#define BITMAP_CHUNK_SIZE 32
#define BITMAP_CHUNK_COUNT(sz) (sz / BITMAP_CHUNK_SIZE + ((sz % BITMAP_CHUNK_SIZE > 0) ? 1 : 0))

Bitmap::Bitmap(uint32_t location, size_t size)
    : m_array((uint32_t*)location)
    , m_size(size)
    , m_memory_size(BITMAP_CHUNK_COUNT(size) * BITMAP_CHUNK_SIZE)
    , m_self_created(false)
{
}

Bitmap::Bitmap(size_t size)
    : m_size(size)
    , m_memory_size(BITMAP_CHUNK_COUNT(size) * BITMAP_CHUNK_SIZE)
    , m_self_created(true)
{
    m_array = new uint32_t[BITMAP_CHUNK_COUNT(size)];
}

Bitmap Bitmap::wrap(uint32_t location, uint32_t size)
{
    return Bitmap(location, size);
}

Bitmap::~Bitmap()
{
    if (m_self_created) {
        delete[] m_array;
    }
}

size_t Bitmap::size() const
{
    return m_size;
}

size_t Bitmap::memory_size() const
{
    return m_memory_size;
}

bool Bitmap::operator[](const size_t index)
{
    return (m_array[index / BITMAP_CHUNK_SIZE] >> (index % BITMAP_CHUNK_SIZE)) & 1;
}

void Bitmap::set_true(const size_t index)
{
    m_array[index / BITMAP_CHUNK_SIZE] |= (1 << (index % BITMAP_CHUNK_SIZE));
}

void Bitmap::set_false(const size_t index)
{
    m_array[index / BITMAP_CHUNK_SIZE] &= ~(1 << (index % BITMAP_CHUNK_SIZE));
}

void Bitmap::clear()
{
    for (uint32_t chunk = 0; chunk < m_size / BITMAP_CHUNK_SIZE; chunk++) {
        m_array[chunk] = 0;
    }
}

void Bitmap::fill()
{
    for (uint32_t chunk = 0; chunk < m_size / BITMAP_CHUNK_SIZE; chunk++) {
        m_array[chunk] = 0xFFFFFFFF;
    }
}

size_t Bitmap::find_first_zero()
{
    for (uint32_t chunk = 0; chunk < m_size / BITMAP_CHUNK_SIZE; chunk++) {
        if (m_array[chunk] != 0xFFFFFFFF) {
            for (uint32_t pos = 0; pos < BITMAP_CHUNK_SIZE; pos++) {
                if (((m_array[chunk] >> pos) & 1) == 0) {
                    return chunk * BITMAP_CHUNK_SIZE + pos;
                }
            }
        }
    }

    return BITMAP_NULL;
}
