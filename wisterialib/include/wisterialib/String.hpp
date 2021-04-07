#pragma once
#include "common.hpp"
#include "Vector.hpp"

class String {

public:
    String() = default;
    String(const String& str);
    String(String&& str);
    String(const char* s);
    String(int);
    String(uint32_t);
    String(char);

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
    void push_back(char c);
    String& operator+=(const String& str);
    String& operator+=(const char* s);
    String& operator+=(char c);
    void pop_back();

    // Comapre operators
    bool operator==(const String& str) const;
    bool operator==(const char* s) const;

    // Other operators
    String operator+(const String& str) const;
    String operator+(const char* cstr) const;

    // Other api functions
    Vector<String> split(const String& del) const;
    char* cstr() const;

private:
    void realloc(size_t);
    void swap();

private:
    char* m_string { nullptr };

    size_t m_size {};
    size_t m_capacity {};
};
