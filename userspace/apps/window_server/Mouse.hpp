#pragma once

class Mouse {
public:
    Mouse() = default;
    Mouse(int fd, int x = 0, int y = 0)
        : m_fd(fd)
        , m_x(x)
        , m_y(y)
    {
    }

    inline int x() const { return m_x; }
    inline int y() const { return m_y; }

    void update_position();

private:
    int m_fd {};
    int m_x {};
    int m_y {};
};