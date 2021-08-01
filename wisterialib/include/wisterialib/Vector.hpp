#pragma once
#include "common.hpp"
#include "extras.hpp"
#include "SimpleIterator.hpp"

template <typename T>
class Vector {
public:
    Vector() = default;

    Vector(size_t size)
        : m_size(size)
        , m_capacity(size)
    {
        m_data = (T*)malloc(m_capacity * sizeof(T));
    }

    Vector(const Vector& v)
    {
        realloc(v.capacity());
        m_size = v.size();
        copy(m_data, v.m_data, m_size);
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
        if (m_data) {
            free(m_data);
            m_data = nullptr;
        }
    };

    Vector& operator=(const Vector& v)
    {
        clear();
        if (v.size() > m_capacity) {
            realloc(v.capacity());
        }
        m_size = v.size();
        copy(m_data, v.m_data, v.size());
        return *this;
    }
    Vector& operator=(Vector&& v)
    {
        if (this != &v) {
            clear();
            m_capacity = v.m_capacity;
            m_size = v.m_size;
            m_data = v.m_data;
            v.m_size = 0;
            v.m_capacity = 0;
            v.m_data = nullptr;
        }
        return *this;
    }

    size_t size() const { return m_size; };
    size_t capacity() const { return m_capacity; }

    T* data() { return m_data; }

    // Element access
    T& operator[](size_t pos) { return m_data[pos]; }
    const T& operator[](size_t pos) const { return m_data[pos]; }

    T& front() { return m_data[0]; }
    const T& front() const { return m_data[0]; }

    T& back() { return m_data[m_size - 1]; }
    const T& back() const { return m_data[m_size - 1]; }

    // Modifiers
    void push_back(T&& val)
    {
        if (m_size >= m_capacity) {
            realloc(m_capacity * 2 + 1);
        }
        new (m_data + m_size++) T(move(val));
    }

    void push_back(const T& val)
    {
        push_back(T(val));
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

    // Iterators
    using ConstIterator = SimpleIterator<const Vector, const T>;
    using Iterator = SimpleIterator<Vector, T>;

    ConstIterator begin() const { return ConstIterator(*this, 0); }
    Iterator begin() { return Iterator(*this, 0); }

    ConstIterator end() const { return ConstIterator(*this, m_size); }
    Iterator end() { return Iterator(*this, m_size); }

private:
    void realloc(size_t size)
    {
        auto new_data = (T*)malloc(size * sizeof(T));

        for (size_t i = 0; i < m_size; i++) {
            new (&new_data[i]) T(move(m_data[i]));
        }

        for (size_t i = 0; i < m_size; i++) {
            m_data[i].~T();
        }

        free(m_data);

        m_data = new_data;
        m_capacity = size;
    }

    void copy(T* to, T* from, size_t len)
    {
        for (size_t i = 0; i < len; i++) {
            new (to) T(*from);
            to++;
            from++;
        }
    }

private:
    T* m_data { nullptr };
    size_t m_size { 0 };
    size_t m_capacity { 0 };
};
