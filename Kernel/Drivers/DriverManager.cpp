#include "DriverManager.hpp"
#include "Base/Driver.hpp"
#include "Base/DriverEntity.hpp"

#include <Drivers/PCI/PCI.hpp>
#include <Drivers/PCI/PCIDevice.hpp>
#include <Libkernel/Logger.hpp>

#include <Macaronlib/Vector.hpp>

namespace Kernel::Drivers {

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
    for (auto& driver : m_drivers) {
        if (driver) {
            driver->install();
        }
    }
}

}