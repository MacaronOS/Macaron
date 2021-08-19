#pragma once

template <typename T>
class Singleton {
public:
    template <typename... Types>
    static bool initialize(Types... args)
    {
        s_t = new T(args...);
        s_initialized = true;
        return s_initialized;
    }
    static bool initialize()
    {
        return initialize<>();
    }
    static T& the()
    {
        // TODO: figure out -fno-rtti and typeid(T).name() to output more detailed error
        if (!s_initialized) {
            while (true) { }
        }
        return *s_t;
    }

private:
    static T* s_t;

public:
    static bool s_initialized;
};