#include "LCDPL111.hpp"

#include <Devices/DeviceManager.hpp>
#include <Memory/VMM/VMM.hpp>
#include <Memory/pmm.hpp>
#include <Tasking/MemoryDescription/MemoryDescription.hpp>
#include <Tasking/MemoryDescription/SharedVMArea.hpp>

namespace Kernel::Devices {

LCDPL111 pl111;

using namespace Tasking;

DriverInstallationResult LCDPL111::try_install()
{
    m_registers_mapper.map_all_registers();
    auto registers = m_registers_mapper.get_register_mask<0>();

    auto width = 1024;
    auto height = 768;
    auto len = width * height * 4 * 2;

    m_framebuffer = PMM::the().allocate_frames(bytes_to_pages(len)) * CPU::page_size();
    m_framebuffer_length = len;

    registers->upper_panel_frame_base_address = m_framebuffer;
    registers->horizontal_axis_panel_control |= (width / 16 - 1) << 2;
    registers->vertical_axis_panel_control |= height - 1;

    auto tmp = registers->control;
    tmp |= pl111_enable;
    tmp |= pl111_24bpp;
    tmp |= pl111_color_mode;
    tmp |= pl111_tft;
    tmp |= pl111_bgr;
    tmp |= pl111_power_enable;
    registers->control = tmp;

    return DriverInstallationResult::Succeeded;
}

void LCDPL111::swap_buffers()
{
    auto registers = m_registers_mapper.get_register_mask<0>();
    auto yoffset = m_buffer_swapper * 768;
    m_buffer_swapper = !m_buffer_swapper;
    registers->upper_panel_frame_base_address = m_framebuffer + yoffset;
}

void LCDPL111::map_framebuffer(void* addr, size_t size)
{
    VMM::the().map_memory(
        (uintptr_t)addr,
        (uintptr_t)m_framebuffer,
        min(m_framebuffer_length, size),
        1);
}

}