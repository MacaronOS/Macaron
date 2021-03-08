#pragma once
#include "Driver.hpp"
#include "DriverEntity.hpp"

#include "../algo/Singleton.hpp"
#include "../assert.hpp"
#include "../errors/KError.hpp"
#include "../types.hpp"

#include <algo/Vector.hpp>

#define MAX_DRIVERS 32

namespace kernel::drivers {

constexpr uint8_t drivers_count = (uint8_t)(DriverEntity::END);

class DriverManager : public Singleton<DriverManager> {
public:
    DriverManager() = default;

    void add_driver(Driver& driver);
    void add_driver(Driver* driver);

    Driver* get_driver(DriverEntity driver_entity);
    void install_all();
    Vector<Driver*> get_by_type(Driver::DriverType);

private:
    Driver* m_drivers[drivers_count] {};
};
}