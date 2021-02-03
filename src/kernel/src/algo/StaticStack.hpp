#pragma once
#include "Array.hpp"
#include "extras.hpp"
#include "../types.hpp"

namespace kernel {

template <typename StaticStack>
class StaticStackIterator {
public:
    using ValueType = typename StaticStack::ValueType;
    using PointerType = ValueType*;
    using ReferenceType = ValueType&;

public:
    StaticStackIterator(PointerType ptr)
        : m_ptr(ptr)
    {
    }

    StaticStackIterator& operator++()
    {
        m_ptr++;
        return *this;
    }
    StaticStackIterator operator++(int)
    {
        StaticStackIterator iter = *this;
        ++(*this);
        return iter;
    }

    StaticStackIterator& operator--()
    {
        m_ptr--;
        return *this;
    }
    StaticStackIterator operator--(int)
    {
        StaticStackIterator iter = *this;
        --(*this);
        return iter;
    }

    ReferenceType operator[](int index)
    {
        return *(m_ptr + index);
    }

    PointerType operator->()
    {
        return m_ptr;
    }

    ReferenceType operator*()
    {
        return *m_ptr;
    }

    bool operator==(const StaticStackIterator& other) const
    {
        return m_ptr == other.m_ptr;
    }

    bool operator!=(const StaticStackIterator& other) const
    {
        return m_ptr != other.m_ptr;
    }

private:
    PointerType m_ptr;
};

template <typename T, size_t S>
class StaticStack {
public:
    using ValueType = T;
    using Iterator = StaticStackIterator<StaticStack<T, S>>;

public:
    constexpr size_t max_size() const { return S; }
    size_t size() const { return m_data_pointer; }
    bool empty() const { return m_data_pointer; }

    void push(const T& val)
    {
        m_data[m_data_pointer++] = val;
    }

    void push(T&& val)
    {
        m_data[m_data_pointer++] = move(val);
    }

    T& top()
    {
        return m_data[m_data_pointer - 1];
    }

    void pop()
    {
        m_data_pointer--;
    }

    T& top_and_pop()
    {
        return m_data[--m_data_pointer];
    }

    Iterator begin()
    {
        return Iterator((T*)&m_data);
    }
    Iterator end()
    {
        return Iterator((T*)((size_t)&m_data + m_data_pointer));
    }

private:
    Array<T, S> m_data;
    size_t m_data_pointer { 0 };
};

}