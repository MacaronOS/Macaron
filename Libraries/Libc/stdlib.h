#pragma once

#include <Macaronlib/Common.hpp>

char* ptsname(int fd);
int ptsname_r(int fd, char* buffer, size_t size);