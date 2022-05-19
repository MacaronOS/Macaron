#include "BGA.hpp"
#include <Devices/DeviceManager.hpp>
#include <Devices/Drivers/x86/PCI/PCI.hpp>
#include <Hardware/x86/Port.hpp>
#include <Memory/VMM/VMM.hpp>
#include <Memory/pmm.hpp>

namespace Kernel::Devices {

BGA bga;

using namespace Memory;

constexpr auto width = 1024;
constexpr auto height = 768;
constexpr auto channels = 4;
constexpr auto buffers = 2;

constexpr uint32_t boch_vbe_io_port_index = 0x01CE;
constexpr uint32_t boch_vbe_io_port_data = 0x01CF;

namespace VBE {
    constexpr uint16_t display_disabled = 0;
    constexpr uint16_t display_enabled = 1 << 0;
    constexpr uint16_t linear_framebuffer_enabled = 1 << 6;
}

enum class IndexRegister : uint16_t {
    ID,
    XRes,
    YRes,
    BPP,
    Enable,
    Bank,
    VirtWidth,
    VirtHeight,
    XOffset,
    YOffset,
};

static inline void bga_write(IndexRegister reg, uint16_t data)
{
    outw(boch_vbe_io_port_index, static_cast<uint16_t>(reg));
    outw(boch_vbe_io_port_data, data);
}

static inline uint16_t bga_read(IndexRegister reg)
{
    outw(boch_vbe_io_port_index, static_cast<uint16_t>(reg));
    return inw(boch_vbe_io_port_data);
}

DriverInstallationResult BGA::try_install()
{
    auto pci = static_cast<PCI*>(DeviceManager::the().find_attached_driver_by_name("PCI"));
    if (!pci) {
        ASSERT_PANIC("[BGA] Installation failed, requires PCI");
        return DriverInstallationResult::Failed;
    }

    auto bga_pci_device = pci->find_pci_device(0x1234, 0x1111);
    if (!bga_pci_device) {
        ASSERT_PANIC("[BGA] Installation failed, no such pci device");
        return DriverInstallationResult::Failed;
    }

    bga_write(IndexRegister::Enable, VBE::display_disabled);
    bga_write(IndexRegister::XRes, width);
    bga_write(IndexRegister::YRes, height);
    bga_write(IndexRegister::VirtWidth, width);
    bga_write(IndexRegister::VirtHeight, height * buffers);
    bga_write(IndexRegister::BPP, 32);
    bga_write(IndexRegister::XOffset, 0);
    bga_write(IndexRegister::YOffset, 0);
    bga_write(IndexRegister::Enable, VBE::display_enabled | VBE::linear_framebuffer_enabled);
    bga_write(IndexRegister::Bank, 0);

    auto physical_address = bga_pci_device->read_base_register(0) & 0xfffffff0;
    PMM::the().occupy_range_sized(physical_address, width * height * channels * buffers);

    m_framebuffer = reinterpret_cast<void*>(physical_address);
    m_framebuffer_length = width * height * channels * buffers;

    return DriverInstallationResult::Succeeded;
}

void BGA::swap_buffers()
{
    bga_write(IndexRegister::YOffset, m_buffer_swapper * 768);
    m_buffer_swapper = !m_buffer_swapper;
}

void BGA::map_framebuffer(void* addr, size_t size)
{
    VMM::the().map_memory(
        (uintptr_t)addr,
        (uintptr_t)m_framebuffer,
        min(m_framebuffer_length, size), Flags::Present | Flags::User | Flags::Write);
}

}