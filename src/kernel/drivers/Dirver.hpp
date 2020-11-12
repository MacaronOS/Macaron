#pragma once
#include "DriverEntity.hpp"
#include "DriverManager.hpp"

namespace kernel::drivers {

class Driver {
public:
    Driver(DriverEntity driver_entity)
        : m_driver_entity(driver_entity)
    {
    }
    ~Driver() = default;

    virtual bool install() { return false; }
    virtual bool uninstall() { return false; }

    DriverEntity driver_entity() { return m_driver_entity; }

private:
    DriverEntity m_driver_entity;
};

}