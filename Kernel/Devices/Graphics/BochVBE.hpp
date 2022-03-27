#pragma once

#include <Devices/Device.hpp>
#include <Drivers/Base/Driver.hpp>
#include <Drivers/Base/DriverEntity.hpp>
#include <Drivers/PCI/PCIDevice.hpp>

#include <Macaronlib/Common.hpp>

namespace Kernel::Devices {

class BochVBE : public Device {
public:
    BochVBE(Drivers::PCIDevice* pci_device)
        : Device(1, 1, DeviceType::Block)
        , m_pci_device(pci_device)
    {
    }
    bool install() override;
    void mmap(void* addr, uint32_t size) override;
    void ioctl(uint32_t request) override;

private:
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
    inline static void write(IndexRegister reg, uint16_t data);
    inline static uint16_t read(IndexRegister reg);

private:
    Drivers::PCIDevice* m_pci_device;
    uint32_t* m_pixels { nullptr };
    uint32_t m_pixels_length { 0 };
    bool cur_buffer { false };
};
}