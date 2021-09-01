#include <Libc/Syscalls.hpp>

void run(const char* path)
{
    if (!fork()) {
        execve(path, nullptr, nullptr);
    }
}

int main()
{
    run("/ext2/System/WindowServer");
    run("/ext2/System/Dock");
    run("/ext2/Applications/Clicker/bin");
    run("/ext2/Applications/Clicker/bin");
    while (true) {
        asm volatile("hlt");
    }
    return 0;
}