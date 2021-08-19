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
    while (true) {
        asm volatile("hlt");
    }
    return 0;
}