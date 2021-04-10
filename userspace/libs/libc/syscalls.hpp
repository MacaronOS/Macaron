/*
 * POSIX SYSCALLS
 */

#pragma once

#include <wisterialib/common.hpp>

int putc(char c);
int exit(int error_code);
int fork();
int open(const char* filename, int flags, uint16_t mode);
int execve(const char* filename, const char* const* argv, const char* const* envp);
void* mmap(void* start, uint32_t length, int prot, int flags, int fd, uint32_t offset);
int ioctl(int fd, unsigned long request);
