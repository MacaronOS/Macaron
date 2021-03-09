#pragma once

#include "PCIDevice.hpp"
#include <drivers/base/Driver.hpp>

namespace kernel::drivers {

class PCIDeviceDriver : public Driver {
public:
    PCIDeviceDriver(DriverEntity driver_entity, PCIDevice* pci_device)
        : Driver(driver_entity)
        , m_pci_device(pci_device)
    {
    }

protected:
    PCIDevice* m_pci_device;
};

}