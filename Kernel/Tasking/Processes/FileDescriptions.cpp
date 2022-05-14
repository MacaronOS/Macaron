#include "FileDescriptions.hpp"

namespace Kernel::Tasking {

KErrorOr<fd_t> FileDescriptions::allocate()
{
    auto alloc_result = m_file_descriptions.allocate();
    if (!alloc_result.object) {
        return KError(EBADFD);
    }
    return alloc_result.id;
}

KErrorOr<FileDescription*> FileDescriptions::lookup(fd_t file_descriptor)
{
    auto file_description = m_file_descriptions.get(file_descriptor);
    if (!file_description) {
        return KError(EBADFD);
    }
    return file_description;
}

FileDescription& FileDescriptions::lookup_no_check(fd_t file_descriptor)
{
    return m_file_descriptions.get_no_check(file_descriptor);
}

KError FileDescriptions::free(fd_t file_descriptor)
{
    if (!m_file_descriptions.deallocate(file_descriptor)) {
        return KError(EBADF);
    }
    return KError(0);
}

}