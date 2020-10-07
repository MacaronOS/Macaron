#pragma once
#include "Array.hpp"
#include "extras.hpp"
#include "types.hpp"

namespace kernel::algorithms {

template <typename T, size_t S>
class StaticStack {
public:
    constexpr size_t max_size() const { return S; }
    size_t size() const { return m_data_pointer; }
    bool empty() const { return m_data_pointer; }

    void push(T& val)
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

private:
    Array<T, S> m_data;
    size_t m_data_pointer { 0 };
};

}