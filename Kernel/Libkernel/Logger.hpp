#pragma once

#include <Macaronlib/String.hpp>

namespace Kernel {

namespace Logger {
    void putc(char c);
    void print(const char* str);
    void print(const String& str);
    void printn(int64_t numb, uint32_t s);
    void printd(int64_t numb);

    class Log {
    public:
        Log() = default;

        friend const Log& operator<<(const Log& log, const String& value);
        friend const Log& operator<<(const Log& log, int value);
        friend const Log& operator<<(const Log& log, char value);
        friend const Log& operator<<(const Log& log, uint32_t value);
        friend const Log& operator<<(const Log& log, uint64_t value);
    };
}

using namespace Logger;

}