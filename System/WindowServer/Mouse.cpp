#include "Mouse.hpp"

#include <Macaronlib/ABI/Syscalls.hpp>

void Mouse::update_position()
{
    m_prev_x = m_x;
    m_prev_y = m_y;

    static MousePacket mouse_buffer[32];
    int bytes = read(m_fd, mouse_buffer, sizeof(mouse_buffer));
    
    for (size_t packet_index = 0; packet_index < bytes / sizeof(MousePacket); packet_index++) {
        m_x += mouse_buffer[packet_index].x_move;
        m_x = max(m_x, 0);
        m_x = min(m_x, m_clipping_width - m_cursor.width() - 1);

        m_y -= mouse_buffer[packet_index].y_move;
        m_y = max(m_y, 0);
        m_y = min(m_y, m_clipping_height - m_cursor.height() - 1);
        
        m_pressed = mouse_buffer[packet_index].left_btn;
    }
}