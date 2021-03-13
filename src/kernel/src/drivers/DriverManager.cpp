#include "DriverManager.hpp"
#include "../errors/KError.hpp"
#include "BochVBE.hpp"
#include "base/Driver.hpp"
#include "base/DriverEntity.hpp"
#include <algo/Vector.hpp>
#include <drivers/pci/PCI.hpp>
#include <drivers/pci/PCIDevice.hpp>

#include <Logger.hpp>

namespace kernel::drivers {

template <>
DriverManager* Singleton<DriverManager>::s_t = nullptr;
template <>
bool Singleton<DriverManager>::s_initialized = false;

void DriverManager::add_driver(Driver& driver)
{
    m_drivers[(uint8_t)driver.driver_entity()] = &driver;
}

void DriverManager::add_driver(Driver* driver)
{
    m_drivers[(uint8_t)driver->driver_entity()] = driver;
}

Driver* DriverManager::get_driver(DriverEntity driver_entity)
{
    if (m_drivers[(uint8_t)driver_entity]) {
        return m_drivers[(uint8_t)driver_entity];
    }

    return nullptr;
}

Vector<Driver*> DriverManager::get_by_type(Driver::DriverType type)
{
    Vector<Driver*> drivers;
    for (size_t driver = 0; driver < drivers_count; driver++) {
        if (m_drivers[driver] && m_drivers[driver]->type() == type) {
            drivers.push_back(m_drivers[driver]);
        }
    }
    return drivers;
}

void DriverManager::install_all()
{
    // install regular drivers
    for (uint8_t driver_index = 0; driver_index < drivers_count; driver_index++) {
        if (m_drivers[driver_index]) {
            m_drivers[driver_index]->install();
        }
    }

    // install pci devices drivers
    auto* pci = reinterpret_cast<PCI*>(get_driver(DriverEntity::PCI));
    if (pci) {
        for (size_t device_index = 0; device_index < pci->devices().size(); device_index++) {
            auto device = pci->devices()[device_index];
            switch (device->vendor_id()) {
            case 0x1234: {
                switch (device->device_id()) {
                case 0x1111: {
                    auto bga = new BochVBE(device);
                    add_driver(bga);
                    bga->install();
                    break;
                }
                }
                break;
            }
            default:
                break;
            }
        }
    }
}

}