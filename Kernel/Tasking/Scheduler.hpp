#pragma once

#include "Process.hpp"
#include "Thread.hpp"

#include <Drivers/PIT.hpp>
#include <Filesystem/VFS/VFS.hpp>
#include <Libkernel/Assert.hpp>
#include <Memory/vmm.hpp>

#include <Macaronlib/List.hpp>
#include <Macaronlib/Singleton.hpp>

namespace Kernel::Tasking {

using namespace Drivers;

class ProcessStorage;
class Process;
class Thread;

class Scheduler : public Singleton<Scheduler>, public TickReciever {
    friend class Process;

public:
    Scheduler();
    bool run();

    Thread* cur_thread();
    Process* cur_process();

    // syscalls handlers
    void sys_exit_handler(int error_code);
    int sys_fork_handler();
    int sys_execve_handler(const char* filename, const char* const* argv, const char* const* envp);

    void create_process(const String& filepath);
    void reschedule();

    void on_tick(Trapframe* tf) override
    {
        reschedule();
    }

public:
    // TODO: support kernel processes / threads
    ProcessStorage* m_process_storage {};
    List<Thread*> m_threads {};
    List<Thread*>::Iterator<ListNode<Thread*>> m_cur_thread { m_threads.end() };
};

}
