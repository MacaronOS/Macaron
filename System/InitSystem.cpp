#include <Libc/Syscalls.hpp>

void run(const char* path)
{
    if (!fork()) {
        execve(path, nullptr, nullptr);
    }
}

int main()
{
    run("/System/WindowServer");
    run("/System/Dock");
    // run("Applications/Clicker/bin");
    // run("Applications/Clicker/bin");
    // run("Applications/Emu/bin");
    run("/Applications/Terminal/bin");
    while (true) {
        asm volatile("hlt");
    }
    return 0;
}