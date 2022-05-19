#pragma once

#include "PCIDevice.hpp"
#include <Devices/Drivers/Driver.hpp>

namespace Kernel::Devices {

class PCIDevice;

class PCI : public Driver {
public:
    REGISTER_DRIVER(PCI)

    PCIDevice* find_pci_device(uint16_t vendor_id, uint16_t device_id);

    // ^Driver
    virtual String driver_name() override
    {
        return "PCI";
    }
    virtual String driver_info() override
    {
        return "MacaronOS x86 PCI driver";
    }

    static bool device_has_functions(uint16_t bus, uint16_t device);
    static uint32_t read(uint16_t bus, uint16_t device, uint16_t func, uint16_t regoff);
    static void write(uint16_t bus, uint16_t device, uint16_t func, uint16_t regoff, uint32_t val);
    static uint32_t get_id(uint16_t bus, uint16_t device, uint16_t func, uint16_t regoff);

protected:
    // ^Driver
    virtual void install() override;

private:
    Vector<PCIDevice> m_devices {};
};

}