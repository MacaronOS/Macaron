#pragma once

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
    friend BufferedLog& operator<<(BufferedLog& log, uint32_t value);
    friend BufferedLog& operator<<(BufferedLog& log, BufferedLogOp op);

private:
    String m_buffer {};
};

static auto Log = BufferedLog();
constexpr auto endl = BufferedLogOp::Endl;