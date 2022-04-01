#include "Log.hpp"

#include <Libc/Syscalls.hpp>
#include <Libc/stdio.h>

BufferedLog& operator<<(BufferedLog& BufferedLog, const String& value)
{
    BufferedLog.m_buffer += value;
    return BufferedLog;
}

BufferedLog& operator<<(BufferedLog& BufferedLog, int value)
{
    BufferedLog.m_buffer += String::From(value);
    return BufferedLog;
}

BufferedLog& operator<<(BufferedLog& BufferedLog, uint32_t value)
{
    BufferedLog.m_buffer += String::From(value);
    return BufferedLog;
}

BufferedLog& operator<<(BufferedLog& BufferedLog, char value)
{
    BufferedLog.m_buffer += String(value);
    return BufferedLog;
}

BufferedLog& operator<<(BufferedLog& log, BufferedLogOp op)
{
    switch (op) {
    case BufferedLogOp::Endl:
        log.m_buffer.push_back('\n');
        write(STDOUT, log.m_buffer.cstr(), log.m_buffer.size());
        log.m_buffer = log.m_begin_with;
        return log;
    }

    return log;
}

BufferedLog::~BufferedLog()
{
    write(STDOUT, m_buffer.cstr(), m_buffer.size());
}
