#pragma once

#include <Libc/Syscalls.hpp>
#include <Libc/stdio.h>
#include <Libsystem/Log.hpp>

class Shell {
public:
    Shell() = default;

    void run();

private:
    void input();
    void process(char buffer[], int sz);
    void output(char buffer[], int sz);
};