#include "cmd.hpp"
#include "../algo/String.hpp"
#include "../fs/vfs.hpp"
#include "../monitor.hpp"
#include "../posix.hpp"

namespace kernel::shell::cmd {
static fs::VFS* s_vfs;

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
    s_vfs = &fs::VFS::the();
}

void clear()
{
    term_init();
}

void ls(const String& path)
{
    auto ld = s_vfs->listdir(path);
    for (size_t i = 0; i < ld.size(); i++) {
        term_print(ld[i]);
        term_print("\n");
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

    char* buff = (char*)kmalloc(file_size.result() + 1);
    auto size = s_vfs->read(fd.result(), buff, file_size.result());
    buff[file_size.result()] = '\0';
    term_print(buff);
    kfree(buff);

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