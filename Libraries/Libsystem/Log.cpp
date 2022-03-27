#include "Log.hpp"
#include "Syscalls.hpp"

#include <Macaronlib/String.hpp>

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
        write_string(log.m_buffer);
        log.m_buffer.clear();
        return log;
    }

    return log;
}

BufferedLog::~BufferedLog()
{
    write_string(m_buffer);
}
