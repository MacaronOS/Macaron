#include <Common.hpp>
#include <Memory.hpp>
#include <String.hpp>
#include <Vector.hpp>

char String::null_char = '\0';

String::String(size_t size)
{
    realloc(size + 1);
}

String::String(const String& str)
{
    *this = str;
}

String::String(String&& str)
{
    *this = move(str);
}

String::String(const char* s)
{
    realloc(4);
    for (size_t i = 0; s[i] != null_char; i++) {
        push_back(s[i]);
    }
}

String::String(char c)
{
    push_back(c);
}

String::~String()
{
    m_size = 0;
    m_capacity = 0;
    if (m_string != &null_char) {
        free(m_string);
        m_string = &null_char;
    }
}

String& String::operator=(const String& str)
{
    if (m_capacity < str.capacity()) {
        realloc(str.capacity());
    }
    memcpy(m_string, str.m_string, str.size() + 1);
    m_size = str.size();
    return *this;
}

String& String::operator=(String&& str)
{
    if (this != &str) {
        if (m_string != &null_char) {
            free(m_string);
        }
        m_string = str.m_string;
        m_size = str.m_size;
        m_capacity = str.m_capacity;
        str.m_string = &null_char;
        str.m_size = 0;
        str.m_capacity = 0;
    }
    return *this;
}

String& String::operator=(const char* s)
{
    for (size_t i = 0; s[i] != null_char; i++) {
        if (i + 1 >= m_capacity) {
            realloc(m_capacity * 2 + 2);
        }
        m_string[i] = s[i];
        m_size++;
    }

    m_string[m_size] = null_char;

    return *this;
}

void String::realloc(size_t new_capacity)
{
    auto new_string = (char*)malloc(new_capacity);
    if (m_string != &null_char) {
        memcpy(new_string, m_string, m_size + 1);
        free(m_string);
    }
    m_string = new_string;
    m_capacity = new_capacity;
}

void String::push_back(char c)
{
    if (m_size + 1 >= m_capacity) {
        realloc(m_capacity * 2 + 2);
    }

    m_string[m_size++] = c;
    m_string[m_size] = null_char;
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
    if (m_size + str.size() + 1 >= m_capacity) {
        realloc((m_capacity + str.capacity()) * 2 + 2);
    }

    memcpy(m_string + m_size, str.m_string, str.m_size + 1);
    m_size += str.m_size;

    return *this;
}

String& String::operator+=(const char* s)
{
    for (size_t i = 0; s[i] != null_char; i++) {
        push_back(s[i]);
    }

    return *this;
}

String& String::operator+=(char c)
{
    push_back(c);
    return *this;
}

void String::pop_back()
{
    m_size = m_size > 0 ? m_size - 1 : 0;
}

void String::reserve(size_t capacity)
{
    capacity++;
    if (m_capacity < capacity) {
        realloc(capacity);
    }
}

void String::clear()
{
    m_size = 0;
    m_string[m_size] = null_char;
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
    for (; s[i] != null_char; i++) {
        if (i >= m_size) {
            return false;
        }
        if (m_string[i] != s[i]) {
            return false;
        }
    }

    if (i != m_size) {
        return false;
    }

    return true;
}

String String::operator+(const String& str) const
{
    auto new_string(*this);
    new_string += str;
    return new_string;
}

String String::operator+(const char* cstr) const
{
    auto new_string(*this);
    for (size_t i = 0; cstr[i] != null_char; i++) {
        new_string.push_back(cstr[i]);
    }

    return new_string;
}

Vector<String> String::split(const String& del) const
{
    Vector<String> result;
    size_t next_string_start = 0;
    for (size_t i = 0; i < m_size; i++) {
        size_t j = i;
        while (j < m_size && j - i < del.size() && m_string[j] == del[j - i]) {
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

const char* String::c_str() const
{
    return m_string;
}

void String::swap()
{
    // TODO: implement something similar to std::reverse
    for (size_t left = 0, right = m_size - 1; left < right; left++, right--) {
        char temp = m_string[left];
        m_string[left] = m_string[right];
        m_string[right] = temp;
    }
}

String String::From(uint32_t num)
{
    String str {};

    do {
        str.push_back('0' + (num % 10));
        num /= 10;
    } while (num);

    str.swap();
    return str;
}

String String::From(int num)
{
    String str {};

    bool negative = false;
    if (num < 0) {
        negative = true;
        num *= -1;
    }
    do {
        str.push_back('0' + (num % 10));
        num /= 10;
    } while (num);

    if (negative) {
        str.push_back('-');
    }

    str.swap();
    return str;
}
