#pragma once

template <typename T>
inline T&& move(T& arg)
{
    return static_cast<T&&>(arg);
}

template <typename T>
struct __RemoveReference {
    using Type = T;
};
template <class T>
struct __RemoveReference<T&> {
    using Type = T;
};
template <class T>
struct __RemoveReference<T&&> {
    using Type = T;
};

template <typename T>
using RemoveReference = typename __RemoveReference<T>::Type;

template <class T>
static constexpr T&& forward(RemoveReference<T>& t) { return static_cast<T&&>(t); }

template <class T>
static constexpr T&& forward(RemoveReference<T>&& t) { return static_cast<T&&>(t); }

extern "C" {

typedef void (*AtExitFunction)(void*);

int __cxa_atexit(AtExitFunction exit_function, void* parameter, void* dso_handle);
void __cxa_finalize(void* dso_handle);

int atexit(void (*handler)());

}