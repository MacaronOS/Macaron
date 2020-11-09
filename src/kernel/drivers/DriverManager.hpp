#pragma once

#include "Dirver.hpp"

#include "../types.hpp"
#include "../assert.hpp"

#define MAX_DRIVERS 32

namespace kernel::drivers {

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

    void add_driver(Driver&);
    void install_all();

private:
    Driver* m_drivers[MAX_DRIVERS] {};
    uint8_t m_drivers_count { 0 };
};

}