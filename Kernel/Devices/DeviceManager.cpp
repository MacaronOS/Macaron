#include "DeviceManager.hpp"
#include "PTY/PTMX.hpp"
#include <Devices/Drivers/Driver.hpp>
#include <Libkernel/Assert.hpp>
#include <Libkernel/Logger.hpp>

namespace Kernel::Devices {

void DeviceManager::register_virtual_devices()
{
    register_device(ptmx);
}

void DeviceManager::acknowledge_driver(Driver* driver)
{
    m_acknowledged_drivers[driver->driver_name()] = driver;
}

void DeviceManager::install_acknowledged_drivers()
{
    static const char* desired_drivers[] = {
        "VgaTUI",
        "UART",
        "PL011",
        "SP804",
        "PL050K",
        "PL050M",
        "PL111",
        "PL181",
        "GICv2",
        "ATA",
        "PS/2-Mouse",
        "PS/2-Keyboard",
        "RTC",
        "PIT",
        "PCI",
        "BGA",
    };

    for (auto desired_driver : desired_drivers) {
        if (m_acknowledged_drivers.contains(desired_driver)) {
            auto driver = m_acknowledged_drivers[desired_driver];
            driver->install();
            m_attached_drivers[desired_driver] = driver;
            Log() << "[DeviceManager] Installed a driver: " << desired_driver << "\n";
            continue;
        }

        Log() << "[DeviceManager] Failed to install a driver: " << desired_driver << "\n";
    }
}

Driver* DeviceManager::find_attached_driver_by_name(const String& name)
{
    if (m_attached_drivers.contains(name)) {
        return m_attached_drivers[name];
    }
    return nullptr;
}

Driver* DeviceManager::find_attached_driver_by_type(DriverType type)
{
    for (auto driver : m_attached_drivers) {
        if (driver.second->driver_type() == type) {
            return driver.second;
        }
    }
    return nullptr;
}

}