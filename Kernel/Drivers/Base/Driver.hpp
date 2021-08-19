#pragma once
#include "DriverEntity.hpp"

namespace Kernel::Drivers {

class Driver {
public:
    enum class DriverType {
        None,
        CharacterDevice,
    };

    Driver(DriverEntity driver_entity, DriverType type = DriverType::None)
        : m_driver_entity(driver_entity)
        , m_type(type)
    {
    }
    ~Driver() = default;

    virtual bool install() { return false; }
    virtual bool uninstall() { return false; }

    DriverEntity driver_entity() { return m_driver_entity; }
    DriverType type() const { return m_type; }

private:
    DriverEntity m_driver_entity;
    DriverType m_type;
};
}