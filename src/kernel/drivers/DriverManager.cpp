#include "DriverManager.hpp"
#include "Dirver.hpp"

namespace kernel::drivers {

void DriverManager::add_driver(Driver& driver)
{
    m_drivers[m_drivers_count++] = driver;
}

void DriverManager::install_all()
{
    for (uint8_t driver_index = 0; driver_index < m_drivers_count; driver_index++) {
        m_drivers[driver_index].install();
    }
}

}