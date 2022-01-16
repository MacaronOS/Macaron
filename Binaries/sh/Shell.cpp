#include "Shell.hpp"
#include <Macaronlib/Memory.hpp>

void Shell::run()
{
    while (true) {
        input();
    }
}

void Shell::input()
{
    char buffer[255] {};
    while (true) {
        int sz = read(STDIN, buffer, sizeof(buffer));
        if (sz > 0) {
            process(buffer, sz);
        }
        sched_yield();
    }
}

void Shell::process(char buffer[], int sz)
{
    char out[255] {};

    char* received = "sh received: ";
    memcpy(out, received, 14);
    memcpy(out + 14, buffer, min(255 - 14, sz));

    output(out, min(255, 14 + sz));
}

void Shell::output(char buffer[], int sz)
{
    write(STDOUT, buffer, sz);
}