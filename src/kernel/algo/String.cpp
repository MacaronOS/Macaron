#include "String.hpp"
#include "Vector.hpp"
#include "../memory/memory.hpp"

namespace kernel::algorithms {

String::String(const String& str)
{
    m_size = str.size();
    m_capacity = str.capacity();

    m_string = new char[m_capacity];
    for (size_t i = 0; i < m_size; i++) {
        m_string[i] = str[i];
    }
}

String::String(String&& str)
{
    m_string = str.m_string;
    m_size = str.m_size;
    m_capacity = str.m_capacity;
    str.m_string = nullptr;
    str.m_size = 0;
    str.m_capacity = 0;
}

String::String(const char* s)
{
    realloc(2);
    for (size_t i = 0; s[i] != 0; i++) {
        push_back(s[i]);
    }
}

String::~String()
{
    delete[] m_string;
}

String& String::operator=(const String& str)
{
    if (m_capacity < str.capacity()) {
        realloc(str.capacity());
    }
    memcpy(m_string, str.m_string, str.size());
    m_size = str.size();
}

String& String::operator=(String&& str)
{
    if (m_string) {
        delete[] m_string;
    }
    m_string = str.m_string;
    m_size = str.m_size;
    m_capacity = str.m_capacity;
    str.m_string = nullptr;
    str.m_size = 0;
    str.m_capacity = 0;
}

String& String::operator=(const char* s)
{
    size_t i = 0;
    for (; s[i] != 0; i++) {
        if (i >= m_capacity) {
            realloc(m_capacity * 2);
        }
        m_string[i] = s[i];
    }
    m_size = i;
}

void String::realloc(size_t new_capacity)
{
    char* new_string = new char[new_capacity];
    memcpy(new_string, m_string, m_size);
    m_string = new_string;
    m_capacity = new_capacity;
}

void String::push_back(const char& c)
{
    if (m_size >= m_capacity) {
        realloc(m_capacity * 2 + 1);
    }

    m_string[m_size++] = c;
}

char& String::operator[](size_t pos)
{
    return m_string[pos];
}

const char& String::operator[](size_t pos) const
{
    return m_string[pos];
}

String& String::operator+=(const String& str)
{
    if (str.size() > m_capacity - m_size) {
        realloc((m_capacity + str.capacity()) * 2);
    }

    for (size_t i = 0; i < str.size(); i++) {
        this->push_back(str[i]);
    }

    return *this;
}

String& String::operator+=(const char* s)
{
    for (size_t i = 0; s[i] != 0; i++) {
        push_back(s[i]);
    }

    return *this;
}

String& String::operator+=(char c)
{
    this->push_back(c);
    return *this;
}

void String::pop_back()
{
    m_size = m_size > 0 ? m_size - 1 : 0;
}

bool String::operator==(const String& str) const
{
    if (str.size() != m_size) {
        return false;
    }

    for (size_t i = 0; i < m_size; i++) {
        if (m_string[i] != str[i]) {
            return false;
        }
    }

    return true;
}

bool String::operator==(const char* s) const
{
    size_t i = 0;
    for (; s[i] != 0; i++) {
        if (m_string[i] != s[i]) {
            return false;
        }
    }

    if (i != m_size) {
        return false;
    }

    return true;
}

Vector<String> String::split(const String& del)
{
    Vector<String> result;
    size_t next_string_start = 0;
    for (size_t i = 0; i < m_size; i++) {
        size_t j = i;
        while (j < m_size && j - i < del.size() && m_string[j] == del[j-i]) {
            j++;
        }
        if (j - i == del.size()) {
            String res;
            for (int k = next_string_start; k < i; k++) {
                res.push_back(m_string[k]);
            }
            result.push_back(move(res));
            i = j - 1;
            next_string_start = j;
        }
    }

    String res;
    for (int k = next_string_start; k < m_size; k++) {
        res.push_back(m_string[k]);
    }
    result.push_back(move(res));

    return result;
}
}