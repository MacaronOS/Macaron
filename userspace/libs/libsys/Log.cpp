#include "Log.hpp"
#include "syscalls.hpp"

#include <wisterialib/String.hpp>

BufferedLog& operator<<(BufferedLog& BufferedLog, const String& value)
{
    BufferedLog.m_buffer += value;
    return BufferedLog;
}

BufferedLog& operator<<(BufferedLog& BufferedLog, int value)
{
    BufferedLog.m_buffer += String(value);
    return BufferedLog;
}

BufferedLog& operator<<(BufferedLog& BufferedLog, uint32_t value)
{
    BufferedLog.m_buffer += String(value);
    return BufferedLog;
}

BufferedLog& operator<<(BufferedLog& BufferedLog, char value)
{
    BufferedLog.m_buffer += String(value);
    return BufferedLog;
}

BufferedLog::~BufferedLog()
{
    write_string(m_buffer);
}
