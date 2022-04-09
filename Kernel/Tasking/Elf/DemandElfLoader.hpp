#pragma once

#include <Macaronlib/Common.hpp>
#include <Tasking/Process.hpp>

namespace Kernel::Tasking {

class Process;

class DemandElfLoader {
public:
    DemandElfLoader(Process& process, const String& filename)
        : m_process(process)
        , m_filename(filename)
    {
    }

    KErrorOr<size_t> load();

private:
    Process& m_process;
    const String& m_filename;
};

}