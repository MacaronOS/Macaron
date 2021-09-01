/*
 * CUSTOM Macaron SYSCALLS
 * POSIX SYSTEM CALLS WRAPPERS ARE THE PART OF LIBC LIBRARY
 */

#pragma once

#include <Macaronlib/ABI/Syscalls.hpp>
#include <Macaronlib/Common.hpp>
#include <Macaronlib/String.hpp>

void write_string(const String&);
CreateBufferResult create_shared_buffer(uint32_t size);
uint32_t get_shared_buffer(uint32_t id);
bool can_read(int pid);
