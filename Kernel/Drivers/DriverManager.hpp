#pragma once
#include "Base/Driver.hpp"
#include "Base/DriverEntity.hpp"

#include <Libkernel/Assert.hpp>
#include <Libkernel/KError.hpp>

#include <Macaronlib/Singleton.hpp>
#include <Macaronlib/Vector.hpp>
#include <Macaronlib/Common.hpp>

#define MAX_DRIVERS 32

namespace Kernel::Drivers {

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