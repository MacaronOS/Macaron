#include "Mouse.hpp"

#include <Macaronlib/ABI/Syscalls.hpp>

void Mouse::pump()
{
    m_prev_x = m_x;
    m_prev_y = m_y;

    static MousePacket mouse_buffer[32];
    int bytes = read(m_fd, mouse_buffer, sizeof(mouse_buffer));
    
    for (size_t packet_index = 0; packet_index < bytes / sizeof(MousePacket); packet_index++) {
        m_x += mouse_buffer[packet_index].x_move;
        m_x = max(m_x, 0);
        m_x = min(m_x, m_clipping_width - cursor().width() - 1);

        m_y -= mouse_buffer[packet_index].y_move;
        m_y = max(m_y, 0);
        m_y = min(m_y, m_clipping_height - cursor().height() - 1);

        auto pressed = mouse_buffer[packet_index].left_btn;
        
        if (!m_pressed && pressed) {
            m_clicks.push_back({ m_x, m_y });
        }
        
        m_pressed = pressed;
    }
}