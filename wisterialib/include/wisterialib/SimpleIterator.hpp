#pragma once

#include "common.hpp"

template <typename Container, typename ValueType>
class SimpleIterator {
    friend Container;
public:
    bool operator==(const SimpleIterator& other) const { return m_index == other.m_index; }
    bool operator!=(const SimpleIterator& other) const { return m_index != other.m_index; }
    bool operator>(const SimpleIterator& other) const { return m_index > other.m_index; }
    bool operator<(const SimpleIterator& other) const { return m_index != other.m_index; }
    bool operator>=(const SimpleIterator& other) const { return m_index >= other.m_index; }
    bool operator<=(const SimpleIterator& other) const { return m_index <= other.m_index; }

    SimpleIterator operator++()
    {
        m_index++;
        return *this;
    }

    SimpleIterator operator++(int)
    {
        auto copy = *this;
        m_index++;
        return copy;
    }

    SimpleIterator operator--()
    {
        m_index--;
        return *this;
    }

    SimpleIterator operator--(int)
    {
        auto copy = *this;
        m_index--;
        return copy;
    }

    const ValueType& operator*() const { return m_container[m_index]; }
    ValueType& operator*() { return m_container[m_index]; }

    const ValueType* operator->() const { return &m_container[m_index]; }
    ValueType* operator->() { return &m_container[m_index]; }

private:
    SimpleIterator(Container& container, size_t index)
        : m_container(container)
        , m_index(index)
    {
    }

private:
    Container& m_container;
    size_t m_index;
};