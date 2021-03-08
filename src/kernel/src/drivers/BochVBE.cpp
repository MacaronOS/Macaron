#include "BochVBE.hpp"
#include "CharacterDevice.hpp"
#include "DriverEntity.hpp"

#include <Logger.hpp>
#include <assert.hpp>
#include <hardware/port.hpp>
#include <memory/vmm.hpp>
#include <types.hpp>

#include <drivers/pci/PCIDeviceDriver.hpp>

namespace kernel::drivers {

constexpr uint32_t BochVBEIoPortIndex = 0x01CE;
constexpr uint32_t BochVBEIoPortData = 0x01CF;

BochVBE::BochVBE(PCIDevice* pci_device)
    : PCIDeviceDriver(DriverEntity::BGA, pci_device)
    , CharacterDevice("bga")
{
}

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

    auto addr = m_pci_device->read_base_register(0) & 0xfffffff0;
    memory::VMM::the().map_virt_to_phys(memory::VMM::the().kernel_page_directory(), addr, addr, 1024 * 768 * 4 * 2);

    auto pixel = reinterpret_cast<uint32_t*>(addr);
    uint32_t i = 0;

    for (; i < 1024 * 768 / 3; i++) {
        pixel[i] = 0xffffffff;
    }

    for (; i < 1024 * 768 / 3 * 2; i++) {
        pixel[i] = 0x000000ff;
    }

    for (; i < 2 * 1024 * 768; i++) {
        pixel[i] = 0x00ff0000;
    }
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

}