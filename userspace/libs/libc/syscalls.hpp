/*
 * POSIX SYSCALLS
 */

#pragma once

#include <wisterialib/common.hpp>
#include <wisterialib/posix/shared.hpp>

int putc(char c);
int exit(int error_code);
int fork();
int read(int fd, void* buf, size_t cnt);
int write(int fd, void* buf, size_t cnt);
int open(const char* filename, int flags, uint16_t mode);
int execve(const char* filename, const char* const* argv, const char* const* envp);
void* mmap(void* start, uint32_t length, int prot, int flags, int fd, uint32_t offset);
int ioctl(int fd, unsigned long request);
int socket(int domain, int type, int protocol);
int bind(int fd, const char* path);
int connect(int fd, const char* path);
int select(int nfds, fd_set* readfds, fd_set* writefds, fd_set* execfds, void* timeout);
int getpid();