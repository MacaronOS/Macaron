#pragma once

#include "Dirver.hpp"

#include "../types.hpp"

#define MAX_DRIVERS 32

namespace kernel::drivers {

class DriverManager {
private:
    Driver m_drivers[MAX_DRIVERS] {};
    uint8_t m_drivers_count { 0 };

public:
    void add_driver(Driver&);
    void install_all();
};

}