#include "PCI.hpp"
#include "PCIDevice.hpp"

#include <Drivers/Base/Driver.hpp>
#include <Drivers/Base/DriverEntity.hpp>
#include <Hardware/x86/Port.hpp>
#include <Libkernel/Logger.hpp>

namespace Kernel::Drivers {

PCI pci;

constexpr uint32_t PCICommandPort = 0xCF8;
constexpr uint32_t PCIDataPort = 0xCFC;

void PCI::initialize()
{
    for (uint8_t bus = 0; bus < 8; bus++) {
        for (uint8_t device = 0; device < 32; device++) {
            uint8_t num_func = DeviceHasFunctions(bus, device) ? 8 : 1;

            for (uint8_t func = 0; func < num_func; func++) {
                uint16_t vendor_id = Read(bus, device, func, 0x00);
                if (vendor_id == 0x0000 || vendor_id == 0xffff) {
                    continue;
                }

                auto dev = new PCIDevice(bus, device, func, vendor_id);
                m_devices.push_back(dev);
            }
        }
    }
}

bool PCI::DeviceHasFunctions(uint16_t bus, uint16_t device)
{
    return Read(bus, device, 0, 0x0e) & (1 << 7);
}

uint32_t PCI::Read(uint16_t bus, uint16_t device, uint16_t func, uint16_t regoff)
{
    outl(PCICommandPort, GetID(bus, device, func, regoff));
    return inl(PCIDataPort) >> (8 * (regoff % 4));
}

void PCI::Write(uint16_t bus, uint16_t device, uint16_t func, uint16_t regoff, uint32_t val)
{
    outl(PCICommandPort, GetID(bus, device, func, regoff));
    outl(PCIDataPort, val);
}

uint32_t PCI::GetID(uint16_t bus, uint16_t device, uint16_t func, uint16_t regoff)
{
    return 1 << 31
        | ((bus & 0xff) << 16)
        | ((device & 0x1f) << 11)
        | ((func & 0x07) << 8)
        | (regoff & 0xfc);
}

}