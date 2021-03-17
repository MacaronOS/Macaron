#pragma once
#include <wisterialib/extras.hpp>
#include <wisterialib/posix/errors.hpp>

namespace kernel {

typedef uint8_t k_error_t;

class KError {
public:
    explicit KError(k_error_t err)
        : m_error(err)
    {
    }

    KError(const KError& err)
        : m_error(err.m_error)
    {
    }

    k_error_t error() const { return m_error; }
    int posix_error() const { return (int)m_error * -1; }
    operator bool() const { return m_error; }

private:
    k_error_t m_error {};
};

template <typename T>
class KErrorOr {
public:
    KErrorOr(const KError& err)
        : m_error(err)
    {
    }
    KErrorOr(KError&& err)
        : m_error(err)
    {
    }
    KErrorOr(T& res)
        : m_result(move(res))
    {
    }

    KErrorOr(T&& res)
        : m_result(move(res))
    {
    }

    T& result() { return m_result; }
    KError error() { return m_error; }

    operator bool() const { return !m_error; }
    T& operator*() { return m_result; }

private:
    KError m_error { KError(0) };
    T m_result;
};

}