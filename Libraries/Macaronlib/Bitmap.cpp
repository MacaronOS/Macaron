#include <Bitmap.hpp>
#include <Common.hpp>
#include <Runtime.hpp>

#define BITMAP_CHUNK_SIZE 32
#define BITMAP_CHUNK_COUNT(sz) (sz / BITMAP_CHUNK_SIZE + ((sz % BITMAP_CHUNK_SIZE > 0) ? 1 : 0))

Bitmap::Bitmap(uintptr_t location, size_t size)
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
    m_array = (uint32_t*)malloc(m_memory_size);
}

Bitmap Bitmap::wrap(uintptr_t location, size_t size)
{
    return Bitmap(location, size);
}

Bitmap::~Bitmap()
{
    if (m_self_created) {
        free(m_array);
    }
}

Bitmap::Bitmap(const Bitmap& other)
{
    *this = other;
}

Bitmap& Bitmap::operator=(const Bitmap& other)
{
    m_self_created = other.m_self_created;
    m_size = other.m_size;
    m_memory_size = other.m_memory_size;
    m_array = (uint32_t*)malloc(m_memory_size);
    for (size_t i = 0; i < BITMAP_CHUNK_COUNT(m_size); i++) {
        m_array[i] = other.m_array[i];
    }
    return *this;
}

Bitmap::Bitmap(Bitmap&& other)
{
    *this = move(other);
}

Bitmap& Bitmap::operator=(Bitmap&& other)
{
    m_self_created = other.m_self_created;
    m_size = other.m_size;
    other.m_size = 0;
    m_memory_size = other.m_memory_size;
    other.m_memory_size = 0;
    m_array = other.m_array;
    other.m_array = nullptr;
    return *this;
}

size_t Bitmap::size() const
{
    return m_size;
}

size_t Bitmap::memory_size() const
{
    return m_memory_size;
}

bool Bitmap::operator[](size_t index)
{
    return (m_array[index / BITMAP_CHUNK_SIZE] >> (index % BITMAP_CHUNK_SIZE)) & 1;
}

void Bitmap::set_true(size_t index)
{
    m_array[index / BITMAP_CHUNK_SIZE] |= (1 << (index % BITMAP_CHUNK_SIZE));
}

void Bitmap::set_false(size_t index)
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

size_t Bitmap::occupy_sequential(size_t size)
{
    for (size_t start = 0; start < m_size; start++) {

        size_t remain = size;
        size_t cur = start;

        for (; cur < m_size && !operator[](cur) && remain; cur++, remain--) { }

        if (!remain) {
            for (size_t i = start; i < start + size; i++) {
                set_true(i);
            }
            return start;
        }

        start = cur;
    }
    return 0;
}
