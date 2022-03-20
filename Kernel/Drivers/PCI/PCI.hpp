#pragma once

#include <Macaronlib/Vector.hpp>
#include <Drivers/Base/Driver.hpp>
#include <Macaronlib/Common.hpp>

namespace Kernel::Drivers {

class PCIDevice;
class PCI {
public:
    static PCI& the()
    {
        static PCI the;
        return the;
    }

    void initialize();

    static bool DeviceHasFunctions(uint16_t bus, uint16_t device);

    static uint32_t Read(uint16_t bus, uint16_t device, uint16_t func, uint16_t regoff);
    static void Write(uint16_t bus, uint16_t device, uint16_t func, uint16_t regoff, uint32_t val);
    
    static uint32_t GetID(uint16_t bus, uint16_t device, uint16_t func, uint16_t regoff);

    const Vector<PCIDevice*>& devices() const { return m_devices; }

private:
    Vector<PCIDevice*> m_devices {};
};

extern PCI pci;

}