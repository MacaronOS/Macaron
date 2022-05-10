#pragma once

#include <Macaronlib/ABI/Errors.hpp>
#include <Macaronlib/Common.hpp>
#include <Macaronlib/Runtime.hpp>

namespace Kernel {

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
    explicit operator int() const { return posix_error(); }

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
    const T& result() const { return m_result; }

    KError error() { return m_error; }

    explicit operator bool() const { return !m_error; }
    explicit operator int() const
    {
        if (m_error) {
            return m_error.posix_error();
        }
        return result();
    }
    T& operator*() { return m_result; }

private:
    KError m_error { KError(0) };
    T m_result;
};

}