#pragma once

#include "Thread.hpp"
#include <FileSystem/Base/File.hpp>

namespace Kernel::Tasking {

class Scheduler;

class Blocker {
    friend class Scheduler;

private:
    Tasking::Thread* thread() { return m_thread; }
    void set_thread(Tasking::Thread* thread) { m_thread = thread; }

private:
    Tasking::Thread* m_thread;
};

class ReadBlocker final : public Blocker {
public:
    ReadBlocker(FileSystem::FileDescription& fd)
        : m_fd(fd)
    {
    }

    bool can_unblock() const;

private:
    FileSystem::FileDescription& m_fd;
};

class WriteBlocker final : public Blocker {
public:
    WriteBlocker(FileSystem::FileDescription& fd)
        : m_fd(fd)
    {
    }

    bool can_unblock() const;

private:
    FileSystem::FileDescription& m_fd;
};

}