#pragma once

#include <Libc/Syscalls.hpp>
#include <Macaronlib/String.hpp>

enum class BufferedLogOp {
    Endl,
};

class BufferedLog {
public:
    BufferedLog() = default;
    ~BufferedLog();

    friend BufferedLog& operator<<(BufferedLog& log, const String& value);
    friend BufferedLog& operator<<(BufferedLog& log, int value);
    friend BufferedLog& operator<<(BufferedLog& log, char value);
    friend BufferedLog& operator<<(BufferedLog& log, BufferedLogOp op);
    friend BufferedLog& operator<<(BufferedLog& log, uint32_t value);
    friend BufferedLog& operator<<(BufferedLog& log, uint64_t value);

private:
    String m_begin_with { String::From(getpid()) + ": " };
    String m_buffer { m_begin_with };
};

static auto Log = BufferedLog();
constexpr auto endl = BufferedLogOp::Endl;