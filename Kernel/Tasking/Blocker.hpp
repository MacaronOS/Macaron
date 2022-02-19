#pragma once

#include "Thread.hpp"
#include <Filesystem/Base/FileDescriptor.hpp>

namespace Kernel {

class Blocker {
public:
    Blocker(Tasking::Thread* thread)
        : m_thread(thread)
    {
    }

    Tasking::Thread* thread() { return m_thread; }

private:
    Tasking::Thread* m_thread;
};

class ReadBlocker final : public Blocker {
public:
    ReadBlocker(FS::FileDescriptor& fd, Tasking::Thread* thread)
        : Blocker(thread)
        , m_fd(fd)
    {
    }

    bool can_unblock() const;

private:
    FS::FileDescriptor& m_fd;
};

}