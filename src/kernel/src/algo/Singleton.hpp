#pragma once
#include "../assert.hpp"

template <typename T>
class Singleton {
public:
    static bool initialize()
    {
        s_t = new T();
        s_initialized = true;
        return s_initialized;
    }
    static T& the()
    {
        // TODO: figure out -fno-rtti and typeid(T).name() to output more detailed error
        if (!s_initialized) {
            ASSERT_PANIC("Singleton referenced before initializing");
        }
        return *s_t;
    }

public:
    static T* s_t;
    static bool s_initialized;
};