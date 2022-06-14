#include "LCDPL111.hpp"

#include <Devices/DeviceManager.hpp>
#include <Memory/VMM/VMM.hpp>
#include <Memory/pmm.hpp>
#include <Tasking/MemoryDescription/MemoryDescription.hpp>
#include <Tasking/MemoryDescription/SharedVMArea.hpp>

namespace Kernel::Devices {

LCDPL111 pl111;

using namespace Tasking;

constexpr auto pl111_physical_address = 0x1c1f0000;

DriverInstallationResult LCDPL111::try_install()
{
    auto pl111_area = kernel_memory_description.allocate_memory_area<SharedVMArea>(
        sizeof(PL111Registers),
        VM_READ | VM_WRITE,
        true);

    if (!pl111_area) {
        ASSERT_PANIC("[DualTimerpl111] Could not allocate pl111 vmarea");
    }

    auto pl111_registers_virtual = pl111_area.result()->vm_start();

    VMM::the().map_memory(
        pl111_registers_virtual,
        pl111_physical_address,
        sizeof(PL111Registers),
        1);

    m_registers = reinterpret_cast<PL111Registers*>(pl111_registers_virtual);

    auto width = 1024;
    auto height = 768;
    auto len = width * height * 4 * 2;

    m_framebuffer = PMM::the().allocate_frames(bytes_to_pages(len)) * CPU::page_size();
    m_framebuffer_length = len;

    m_registers->upper_panel_frame_base_address = m_framebuffer;
    m_registers->horizontal_axis_panel_control |= (width / 16 - 1) << 2;
    m_registers->vertical_axis_panel_control |= height - 1;

    auto tmp = m_registers->control;
    tmp |= pl111_enable;
    tmp |= pl111_24bpp;
    tmp |= pl111_color_mode;
    tmp |= pl111_tft;
    tmp |= pl111_bgr;
    tmp |= pl111_power_enable;
    m_registers->control = tmp;

    return DriverInstallationResult::Succeeded;
}

void LCDPL111::swap_buffers()
{
    auto yoffset = m_buffer_swapper * 768;
    m_buffer_swapper = !m_buffer_swapper;
    m_registers->upper_panel_frame_base_address = m_framebuffer + yoffset;
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