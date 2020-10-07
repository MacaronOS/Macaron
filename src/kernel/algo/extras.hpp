#pragma once

template<typename T>
inline T&& move(T& arg)
{
    return static_cast<T&&>(arg);
}