/*
 * CUSTOM WISTERIA SYSCALLS
 * POSIX SYSTEM CALLS WRAPPERS ARE THE PART OF LIBC LIBRARY
 */

#pragma once

#include <Wisterialib/ABI/Syscalls.hpp>
#include <Wisterialib/Common.hpp>
#include <Wisterialib/String.hpp>

void write_string(const String&);

CreateBufferResult create_shared_buffer(uint32_t size);
uint32_t get_shared_buffer(uint32_t id);

bool can_read(int pid);
