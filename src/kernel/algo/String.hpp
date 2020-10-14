#pragma once
#include "../types.hpp"
#include "Vector.hpp"

namespace kernel::algorithms {

class String {

public:
    String() = default;
    String(const String& str);
    String(String&& str);
    String(const char* s);

    ~String();

    String& operator=(const String& str);
    String& operator=(String&& str);
    String& operator=(const char* s);

    size_t size() const { return m_size; }
    size_t capacity() const { return m_capacity; }

    // Element access
    char& operator[](size_t pos);
    const char& operator[](size_t pos) const;

    // Modifiers
    void push_back(const char&);
    String& operator+=(const String& str);
    String& operator+=(const char* s);
    String& operator+=(char c);
    void pop_back();

    // Comapre operators
    bool operator==(const String& str) const;
    bool operator==(const char* s) const;

    // Other
    Vector<String> split(const String& del) const;

private:
    void realloc(size_t);

private:
    char* m_string {};

    size_t m_size {};
    size_t m_capacity{};
};

}