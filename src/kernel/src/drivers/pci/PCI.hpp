#pragma once

#include <algo/Vector.hpp>
#include <drivers/base/Driver.hpp>
#include <types.hpp>

namespace kernel::drivers {

class PCIDevice;
class PCI : public Driver {
public:
    PCI();
    bool install() override;

    static bool DeviceHasFunctions(uint16_t bus, uint16_t device);

    static uint32_t Read(uint16_t bus, uint16_t device, uint16_t func, uint16_t regoff);
    static void Write(uint16_t bus, uint16_t device, uint16_t func, uint16_t regoff, uint32_t val);
    
    static uint32_t GetID(uint16_t bus, uint16_t device, uint16_t func, uint16_t regoff);

    const Vector<PCIDevice*>& devices() const { return m_devices; }

private:
    Vector<PCIDevice*> m_devices {};
};

}