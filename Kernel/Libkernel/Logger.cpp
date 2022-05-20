#include "Logger.hpp"
#include <Devices/Device.hpp>

namespace Kernel {

static Device* s_console;

namespace Logger {
    void initialize(Device* console_device)
    {
        s_console = console_device;
    }

    void putc(char c)
    {
        if (!s_console) {
            return;
        }
        FileDescription fd;
        s_console->write(&c, 1, fd);
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

    void printd(uint32_t numb)
    {
        char buffer[15];
        int pos = 0;

        do {
            buffer[pos++] = numb % 10 + '0';
            numb /= 10;
        } while (numb);

        while (pos) {
            putc((char)buffer[pos - 1]);
            pos--;
        }
    }

    const Log& operator<<(const Log& log, const String& value)
    {
        print(value);
        return log;
    }

    const Log& operator<<(const Log& log, uint32_t value)
    {
        printd(value);
        return log;
    }

    const Log& operator<<(const Log& log, uint64_t value)
    {
        printd((uint32_t)value);
        return log;
    }

    const Log& operator<<(const Log& log, int value)
    {
        printd((int64_t)value);
        return log;
    }

    const Log& operator<<(const Log& log, char value)
    {
        putc(value);
        return log;
    }
}
}