#include "PCIDevice.hpp"
#include "PCI.hpp"

#include <Libkernel/Logger.hpp>
#include <Drivers/Base/Driver.hpp>
#include <Drivers/Base/DriverEntity.hpp>
#include <Hardware/Port.hpp>

namespace Kernel::Drivers {

PCIDevice::PCIDevice(uint16_t bus, uint16_t device, uint16_t function, uint16_t vendor_id)
    : m_bus(bus)
    , m_device(device)
    , m_function(function)
    , m_vendor_id(vendor_id)
{
    // setting device information
    if (!m_vendor_id) {
        m_vendor_id = read(0x00);
    }
    m_interrupt = read(0x3c);
    m_vendor_id = vendor_id;
    m_device_id = read(0x02);
    m_class_id = read(0x0b);
    m_subclass_id = read(0x0a);
    m_interface_id = read(0x09);
    m_revision = read(0x08);

    // setting base registers
    uint32_t header_type = read(0x0e) & 0x7f;
    int bars = 6 - (4 * header_type);

    for (int bar = 0; bar < bars; bar++) {
        uint32_t bar_value = read_base_register(bar);
        m_base_adress_registers[bar].type = static_cast<BaseAdressRegisterType>(bar_value & 1);

        if (m_base_adress_registers[bar].type == BaseAdressRegisterType::IO) {
            m_base_adress_registers[bar].address = reinterpret_cast<uint8_t*>(bar_value & ~0x3);
            m_base_adress_registers[bar].prefetchable = false;
        }

        // TODO: support MemoryMapping registers
    }
}

uint32_t PCIDevice::read_base_register(uint8_t bar)
{
    return read(0x10 + 4 * bar);
}

uint32_t PCIDevice::read(uint16_t regoff)
{
    return PCI::Read(m_bus, m_device, m_function, regoff);
}

}