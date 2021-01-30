#pragma once
#include "Dirver.hpp"
#include "DriverEntity.hpp"

#include "../assert.hpp"
#include "../types.hpp"
#include "../errors/KError.hpp"

#define MAX_DRIVERS 32

namespace kernel::drivers {

constexpr uint8_t drivers_count = (uint8_t)(DriverEntity::END);
class Driver;

class DriverManager {
public:
    DriverManager() = default;

    static DriverManager* s_dm;
    static bool initialized;
    static bool initialize()
    {
        s_dm = new DriverManager();
        DriverManager::initialized = true;
        return DriverManager::initialized;
    }
    static DriverManager& the()
    {
        if (!DriverManager::initialized) {
            ASSERT_PANIC("Driver manager referenced before initializing");
        }
        return *s_dm;
    }

    void add_driver(Driver& driver);
    Driver* get_driver(DriverEntity driver_entity);
    void install_all();

private:
    Driver* m_drivers[drivers_count] {};
};
}