#pragma once

#include "Thread.hpp"
#include <FileSystem/Base/File.hpp>

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
    ReadBlocker(FileSystem::FileDescription& fd, Tasking::Thread* thread)
        : Blocker(thread)
        , m_fd(fd)
    {
    }

    bool can_unblock() const;

private:
    FileSystem::FileDescription& m_fd;
};

class WriteBlocker final : public Blocker {
public:
    WriteBlocker(FileSystem::FileDescription& fd, Tasking::Thread* thread)
        : Blocker(thread)
        , m_fd(fd)
    {
    }

    bool can_unblock() const;

private:
    FileSystem::FileDescription& m_fd;
};

}