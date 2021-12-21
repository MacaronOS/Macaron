#pragma once

#include "Common.hpp"

template <size_t BuffSize>
class Ringbuffer {
public:
    Ringbuffer()
    {
        m_buffer = (uint8_t*)malloc(BuffSize);
    }

    size_t read(uint8_t* buffer, size_t size)
    {
        size_t i = 0;

        if (start > end) {
            for (; start < BuffSize && i < size; i++, start++) {
                buffer[i] = m_buffer[start];
            }
            if (start == BuffSize) {
                start = 0;
            }
        }

        for (; start < end && i < size; i++, start++) {
            buffer[i] = m_buffer[start];
        }

        return i;
    }

    size_t read_from(uint8_t* buffer, size_t offset, size_t size)
    {
        offset %= BuffSize;
        size_t i = 0;

        if (offset > end) {
            for (; offset < BuffSize && i < size; i++, offset++) {
                buffer[i] = m_buffer[offset];
            }
            offset = 0;
        }

        for (; offset < end && i < size; i++, offset++) {
            buffer[i] = m_buffer[offset];
        }

        return i;
    }

    size_t write(const uint8_t* buffer, size_t size)
    {
        size_t i = 0;

        if (end >= start) {
            for (; end < BuffSize && i < size; end++, i++) {
                m_buffer[end] = buffer[i];
            }
            if (end == BuffSize) {
                end = 0;
            }
        }

        for (; end < start && i < size; end++, i++) {
            m_buffer[end] = buffer[i];
        }

        return i;
    }

    void write_force(const uint8_t* buffer, size_t size)
    {
        for (uint32_t bytes_written = 0; bytes_written < size; bytes_written++) {
            m_buffer[end] = buffer[bytes_written];
            end = (end + 1) % BuffSize;
        }
    }

    size_t space_to_read() const
    {
        if (end < start) {
            return BuffSize - start + end;
        }
        return end - start;
    }

    size_t space_to_read_from(size_t offset) const
    {
        offset %= BuffSize;
        if (offset < start) {
            return BuffSize - offset + end;
        }
        return end - offset;
    }

    size_t space_to_write() const
    {
        if (end < start) {
            return end - start;
        }
        return BuffSize - end + start;
    }

private:
    uint8_t* m_buffer {};
    size_t start {};
    size_t end {};
};