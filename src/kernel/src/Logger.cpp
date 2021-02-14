#pragma once

#include <algo/String.hpp>
#include <drivers/DriverManager.hpp>
#include <drivers/Uart.hpp>

namespace kernel {

using namespace drivers;

namespace Logger {
    void putc(char c)
    {
        static_cast<Uart*>(DriverManager::the().get_driver(DriverEntity::Uart))->send(c);
    }

    void print(const char* str)
    {
        for (size_t i = 0; str[i] != '\0'; i++) {
            putc(str[i]);
        }
    }

    void print(const String& str)
    {
        for (size_t i = 0; i < str.size(); i++) {
            putc(str[i]);
        }
    }

    void printn(int64_t numb, uint32_t s)
    {
        bool negative = 0;

        if (numb < 0) {
            numb *= -1;
            negative = 1;
        }

        char buffer[sizeof(uint32_t) * 8 + 2];
        int pos = 0;

        do {
            buffer[pos++] = numb % s + '0';
            numb /= s;
        } while (numb);

        if (negative) {
            putc('-');
        }

        while (pos) {
            putc((char)buffer[pos - 1]);
            pos--;
        }
    }

    void printd(int64_t numb)
    {
        printn(numb, 10);
    }
}
}