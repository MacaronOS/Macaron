#pragma once

#include <Devices/Drivers/Generic/InterruptTimer.hpp>
#include <FileSystem/VFS/VFS.hpp>
#include <Libkernel/Assert.hpp>
#include <Tasking/Blocker.hpp>
#include <Tasking/Processes/Process.hpp>

#include <Macaronlib/List.hpp>

namespace Kernel::Tasking {

using namespace Devices;

class Scheduler : public InterruptTimerCallback {
    friend class Process;
    using Iterator = List<Thread*>::Iterator;

public:
    static auto& the()
    {
        static Scheduler the;
        return the;
    }

    inline bool running() const { return m_running; }
    Thread& current_thread();
    Process& current_process();

    void initialize();
    void run();
    void reschedule();

    void on_interrupt_timer(Trapframe* tf) override
    {
        reschedule();
    }

    template <typename BlockerType>
    void block_current_thread_with(BlockerType&& blocker)
    {
        blocker.set_thread(&current_thread());
        if constexpr (IsSame<BlockerType, ReadBlocker>) {
            m_read_blockers.push_back(blocker);
        } else if constexpr (IsSame<BlockerType, WriteBlocker>) {
            m_write_blockers.push_back(blocker);
        }
        block_current_thread();
    }

private:
    void unblock_threads()
    {
        unblock_threads_on<ReadBlocker>();
        unblock_threads_on<WriteBlocker>();
    }

    template <typename BlockerType>
    void unblock_threads_on()
    {
        if constexpr (IsSame<BlockerType, ReadBlocker>) {
            unblock_blocker_list<List<ReadBlocker>>(m_read_blockers);
        } else if constexpr (IsSame<BlockerType, WriteBlocker>) {
            unblock_blocker_list<List<WriteBlocker>>(m_write_blockers);
        }
    }

    template <typename BlockerList>
    void unblock_blocker_list(BlockerList& blocker_list)
    {
        auto blocker = blocker_list.rbegin();
        while (blocker != blocker_list.rend()) {
            if ((*blocker).can_unblock()) {
                unblock_blocker(*blocker);
                blocker = blocker_list.remove(blocker);
                continue;
            }
            --blocker;
        }
    }

    void unblock_blocker(Blocker&);
    void block_current_thread();

    void add_thread_for_schedulling(Thread* thread);
    void remove_thread_from_schedulling(Thread* thread);

    Iterator find_next_thread();
    void prepare_switching_to_the_next_thread(Iterator next_thread);

private:
    bool m_running {};
    InterruptTimer* m_interrupt_timer;
    List<Thread*> m_schedulling_threads {};
    Iterator m_current_thread {};
    List<ReadBlocker> m_read_blockers {};
    List<WriteBlocker> m_write_blockers {};
};

}
