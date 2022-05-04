#include "BochVBE.hpp"

#include <Drivers/Base/DriverEntity.hpp>
#include <Hardware/Port.hpp>
#include <Libkernel/Assert.hpp>
#include <Libkernel/Logger.hpp>
#include <Memory/pmm.hpp>
#include <Memory/vmm.hpp>
#include <Tasking/Process.hpp>
#include <Tasking/Scheduler.hpp>

#include <Macaronlib/ABI/Syscalls.hpp>
#include <Macaronlib/Common.hpp>

namespace Kernel::Devices {

BochVBE bga;

using namespace Memory;
using namespace Tasking;

constexpr uint32_t BochVBEIoPortIndex = 0x01CE;
constexpr uint32_t BochVBEIoPortData = 0x01CF;

namespace VBE {
    constexpr uint16_t DisplayDisabled = 0;
    constexpr uint16_t DisplayEnabled = 1 << 0;
    constexpr uint16_t LinearFrameBufferEnabled = 1 << 6;
}

bool BochVBE::install()
{
    write(IndexRegister::Enable, VBE::DisplayDisabled);
    write(IndexRegister::XRes, 1024);
    write(IndexRegister::YRes, 768);
    write(IndexRegister::VirtWidth, 1024);
    write(IndexRegister::VirtHeight, 768 * 2);
    write(IndexRegister::BPP, 32);
    write(IndexRegister::XOffset, 0);
    write(IndexRegister::YOffset, 0);
    write(IndexRegister::Enable, VBE::DisplayEnabled | VBE::LinearFrameBufferEnabled);
    write(IndexRegister::Bank, 0);

    auto physical_address = m_pci_device->read_base_register(0) & 0xfffffff0;
    PMM::the().occupy_range_sized(physical_address, 1024 * 768 * 4 * 2);

    m_pixels = reinterpret_cast<uint32_t*>(physical_address);
    m_pixels_length = 1024 * 768 * 4 * 2;

    return true;
}

void BochVBE::mmap(void* addr, uint32_t size)
{
    VMM::the().map_memory(
        (uintptr_t)addr,
        (uintptr_t)m_pixels,
        min(m_pixels_length, size), Flags::Present | Flags::User | Flags::Write);
}

inline void BochVBE::write(IndexRegister reg, uint16_t data)
{
    outw(BochVBEIoPortIndex, static_cast<uint16_t>(reg));
    outw(BochVBEIoPortData, data);
}

inline uint16_t BochVBE::read(IndexRegister reg)
{
    outw(BochVBEIoPortIndex, static_cast<uint16_t>(reg));
    return inw(BochVBEIoPortData);
}

void BochVBE::ioctl(uint32_t request)
{
    if (request == BGA_SWAP_BUFFERS) {
        cur_buffer = !cur_buffer;
        write(IndexRegister::YOffset, cur_buffer * 768);
        return;
    }
}

}