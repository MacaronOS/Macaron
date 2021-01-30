#pragma once
#include "Dirver.hpp"
#include "DriverEntity.hpp"

#include "../assert.hpp"
#include "../types.hpp"
#include "../errors/KError.hpp"
#include "../algo/Singleton.hpp"

#define MAX_DRIVERS 32

namespace kernel::drivers {

constexpr uint8_t drivers_count = (uint8_t)(DriverEntity::END);
class Driver;

class DriverManager : public Singleton<DriverManager> {
public:
    DriverManager() = default;

    void add_driver(Driver& driver);
    Driver* get_driver(DriverEntity driver_entity);
    void install_all();

private:
    Driver* m_drivers[drivers_count] {};
};
}