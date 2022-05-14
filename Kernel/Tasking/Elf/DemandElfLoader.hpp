#pragma once

#include <Macaronlib/String.hpp>
#include <Libkernel/KError.hpp>

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