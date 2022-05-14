#pragma once

#include <Filesystem/Base/File.hpp>
#include <Filesystem/VFS/VFS.hpp>
#include <Libkernel/KError.hpp>
#include <Macaronlib/ObjectPool.hpp>

namespace Kernel::Tasking {

using namespace FileSystem;

class FileDescriptions {
public:
    KErrorOr<fd_t> allocate();
    KErrorOr<FileDescription*> lookup(fd_t file_descriptor);
    FileDescription& lookup_no_check(fd_t file_descriptor);
    KError free(fd_t file_descriptor);

private:
    ObjectPool<FileDescription, 32> m_file_descriptions {};
};

}