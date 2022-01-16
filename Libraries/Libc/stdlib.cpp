#include "stdlib.h"

static char ptsname_buf[32];
char* ptsname(int fd)
{
    if (ptsname_r(fd, ptsname_buf, sizeof(ptsname_buf)) < 0) {
        return nullptr;
    }
    return ptsname_buf;
}