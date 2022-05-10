#pragma once

#include "Common.hpp"
#include "Runtime.hpp"
#include "String.hpp"
#include "Vector.hpp"

class Encoder {
public:
    Encoder() = default;

    void push(int val)
    {
        m_buffer.push_back((unsigned char)val);
        m_buffer.push_back((unsigned char)(val >> 8));
        m_buffer.push_back((unsigned char)(val >> 16));
        m_buffer.push_back((unsigned char)(val >> 24));
    }

    void push(unsigned char val)
    {
        m_buffer.push_back((unsigned char)val);
    }

    void push(const String& string)
    {
        push((int)string.size());
        push((int)string.capacity());
        for (unsigned char c : string) {
            push(c);
        }
    }

    unsigned char* data() { return m_buffer.data(); }
    size_t size() const { return m_buffer.size(); }

    Vector<unsigned char> done() { return move(m_buffer); }

private:
    Vector<unsigned char> m_buffer {};
};

class Decoder {
public:
    Decoder(const Vector<unsigned char>& data)
        : m_data(data)
    {
    }

    int get_int()
    {
        return (int)m_data[m_offset++] | ((int)m_data[m_offset++]) << 8 | ((int)m_data[m_offset++]) << 16 | ((int)m_data[m_offset++]) << 24;
    }

    unsigned char get_char()
    {
        return m_data[m_offset++];
    }

    String get_String()
    {
        size_t size = get_int();
        size_t capacity = get_int();
        String string {};

        string.reserve(capacity);

        for (size_t _ = 0; _ < size; _++) {
            string.push_back(get_char());
        }

        return string;
    }

    inline void skip(size_t bytes) { m_offset += bytes; }

private:
    const Vector<unsigned char>& m_data;
    size_t m_offset {};
};