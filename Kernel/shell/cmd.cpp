#include "cmd.hpp"

#include <Wisterialib/String.hpp>
#include <Wisterialib/posix/defines.hpp>

#include <Filesystem/VFS/VFS.hpp>
#include <Libkernel/Graphics/VgaTUI.hpp>

namespace Kernel::shell::cmd {
static FS::VFS* s_vfs;

static inline bool check_path_exists(const String& path)
{
    auto fd = s_vfs->open(path, 11);
    if (!fd) {
        return false;
    }
    s_vfs->close(fd.result());
    return true;
}

void init()
{
    s_vfs = &FS::VFS::the();
}

void clear()
{
    VgaTUI::Initialize();
}

void ls(const String& path)
{
    auto ld = s_vfs->listdir(path);
    for (size_t i = 0; i < ld.size(); i++) {
        VgaTUI::Print(ld[i]);
        VgaTUI::Print("\n");
    }
}

void cat(const String& path)
{
    if (!path.size()) {
        return;
    }

    auto fd = s_vfs->open(path, 11);
    if (!fd) {
        return;
    }

    auto file_size = s_vfs->file_size(fd.result());
    if (!file_size) {
        return;
    }

    char* buff = (char*)malloc(file_size.result() + 1);
    auto size = s_vfs->read(fd.result(), buff, file_size.result());
    buff[file_size.result()] = '\0';
    VgaTUI::Print(buff);
    free(buff);

    s_vfs->close(fd.result());
}

void echo(const String& msg, const String& path, bool append)
{
    if (!path.size()) {
        return;
    }

    auto fd = s_vfs->open(path, 11, O_CREAT);
    if (fd) {
        if (append) {
            s_vfs->lseek(fd.result(), 0, 2);
            s_vfs->write(fd.result(), msg.cstr(), msg.size());
        } else {
            s_vfs->write(fd.result(), msg.cstr(), msg.size());
            s_vfs->truncate(fd.result(), msg.size());
        }
        s_vfs->close(fd.result());
    }
}

void touch(const String& path)
{
    if (!path.size()) {
        return;
    }

    auto fd = s_vfs->open(path, 1, O_CREAT);
    if (fd) {
        s_vfs->close(fd.result());
    }
}

}