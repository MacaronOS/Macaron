#include "PCI.hpp"

#include <Hardware/x86/Port.hpp>

namespace Kernel::Devices {

PCI pci;

constexpr uint32_t PCICommandPort = 0xCF8;
constexpr uint32_t PCIDataPort = 0xCFC;

PCIDevice* PCI::find_pci_device(uint16_t vendor_id, uint16_t device_id)
{
    // TODO: faster lookup
    for (auto& device : m_devices) {
        if (device.vendor_id() == vendor_id && device.device_id() == device_id) {
            return &device;
        }
    }
    return nullptr;
}

void PCI::install()
{
    for (uint8_t bus = 0; bus < 8; bus++) {
        for (uint8_t device = 0; device < 32; device++) {
            uint8_t num_func = device_has_functions(bus, device) ? 8 : 1;

            for (uint8_t func = 0; func < num_func; func++) {
                uint16_t vendor_id = read(bus, device, func, 0x00);
                if (vendor_id == 0x0000 || vendor_id == 0xffff) {
                    continue;
                }
                m_devices.push_back(PCIDevice(bus, device, func, vendor_id));
            }
        }
    }
}

bool PCI::device_has_functions(uint16_t bus, uint16_t device)
{
    return read(bus, device, 0, 0x0e) & (1 << 7);
}

uint32_t PCI::read(uint16_t bus, uint16_t device, uint16_t func, uint16_t regoff)
{
    outl(PCICommandPort, get_id(bus, device, func, regoff));
    return inl(PCIDataPort) >> (8 * (regoff % 4));
}

void PCI::write(uint16_t bus, uint16_t device, uint16_t func, uint16_t regoff, uint32_t val)
{
    outl(PCICommandPort, get_id(bus, device, func, regoff));
    outl(PCIDataPort, val);
}

uint32_t PCI::get_id(uint16_t bus, uint16_t device, uint16_t func, uint16_t regoff)
{
    return 1 << 31
        | ((bus & 0xff) << 16)
        | ((device & 0x1f) << 11)
        | ((func & 0x07) << 8)
        | (regoff & 0xfc);
}

}