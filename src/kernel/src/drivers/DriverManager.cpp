#include "DriverManager.hpp"
#include "DriverEntity.hpp"
#include "Dirver.hpp"
#include "../errors/KError.hpp"

namespace kernel::drivers {

template <>
DriverManager* Singleton<DriverManager>::s_t = nullptr;
template <>
bool Singleton<DriverManager>::s_initialized = false;

void DriverManager::add_driver(Driver& driver)
{
    m_drivers[(uint8_t)driver.driver_entity()] = &driver;
}

Driver* DriverManager::get_driver(DriverEntity driver_entity) {
    if (m_drivers[(uint8_t)driver_entity]) {
        return m_drivers[(uint8_t)driver_entity];
    }

    return nullptr;
}

void DriverManager::install_all()
{
    for (uint8_t driver_index = 0; driver_index < drivers_count; driver_index++) {
        if (m_drivers[driver_index]) {
            m_drivers[driver_index]->install();
        }
    }
}

}