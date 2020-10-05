#pragma once
#include "extras.hpp"

#include "../memory/memory.hpp"
#include "../types.hpp"

namespace kernel::algorithms {

template <typename T>
class Vector {
public:
    Vector() = default;

    Vector(size_t size)
        : m_size(size)
        , m_capacity(size)
    {
        m_data = new T[m_capacity];
    }

    Vector(const Vector& v)
    {
        realloc(v.capacity());
        for (size_t i = 0; i < v.size(); i++) {
            m_data[i] = v[i];
        }
        m_size = v.size();
    }

    Vector(Vector&& v)
    {
        m_capacity = v.m_capacity;
        m_size = v.m_size;
        m_data = v.m_data;
        v.m_size = 0;
        v.m_capacity = 0;
        v.m_data = nullptr;
    }

    ~Vector()
    {
        clear();
        ::operator delete(m_data, m_capacity * sizeof(T));
    };

    Vector& operator=(const Vector& v)
    {
        clear();
        if (v.size() > m_capacity) {
            realloc(v.capacity());
        }
        for (size_t i = 0; i < v.size(); i++) {
            m_data[i] = v[i];
        }
    }
    Vector& operator=(Vector&& v)
    {
        clear();
        m_capacity = v.m_capacity;
        m_size = v.m_size;
        m_data = v.m_data;
        v.m_size = 0;
        v.m_capacity = 0;
        v.m_data = nullptr;
    }

    size_t size() const { return m_size; };
    size_t capacity() const { return m_capacity; }

    // Element access
    T& operator[](size_t pos) { return m_data[pos]; }
    const T& operator[](size_t pos) const { return m_data[pos]; }

    T& front() { return m_data[0]; }
    const T& front() const { return m_data[0]; }

    T& back() { return m_data[m_size - 1]; }
    const T& back() const { return m_data[m_size - 1]; }

    // Modifiers
    void push_back(const T& val)
    {
        if (m_size >= m_capacity) {
            realloc(m_capacity * 2);
        }
        m_data[m_size++] = val;
    }

    void push_back(T&& val)
    {
        if (m_size >= m_capacity) {
            realloc(m_capacity * 2);
        }
        m_data[m_size++] = move(val);
    }

    void pop_back()
    {
        if (m_size > 0) {
            m_data[--m_size].~T();
        }
    }

    void clear()
    {
        for (size_t i = 0; i < m_size; i++) {
            m_data[i].~T();
        }
        m_size = 0;
    }

    // Compare operators
    bool operator==(const Vector& v) const
    {
        if (m_size != v.size()) {
            return false;
        }
        for (size_t i = 0; i < v.size(); i++) {
            if ((*this)[i] != v[i]) {
                return false;
            }
        }
        return true;
    }

private:
    void realloc(size_t size)
    {
        T* new_data = (T*)::operator new(size * sizeof(T));
        for (size_t i = 0; i < m_size; i++) {
            new_data[i] = move(m_data[i]);
        }

        for (size_t i = 0; i < m_size; i++) {
            m_data[i].~T();
        }

        ::operator delete(m_data, m_capacity * sizeof(T));
        m_data = new_data;
        m_capacity = size;
    }

private:
    T* m_data { nullptr };
    size_t m_size { 0 };
    size_t m_capacity { 0 };
};

}