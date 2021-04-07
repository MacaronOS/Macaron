#pragma once

#include <wisterialib/String.hpp>

class BufferedLog {
public:
    BufferedLog() = default;
    ~BufferedLog();

    friend BufferedLog& operator<<(BufferedLog& log, const String& value);
    friend BufferedLog& operator<<(BufferedLog& log, int value);
    friend BufferedLog& operator<<(BufferedLog& log, char value);
    friend BufferedLog& operator<<(BufferedLog& log, uint32_t value);

private:
    String m_buffer {};
};

#define Log auto log = BufferedLog(); log
