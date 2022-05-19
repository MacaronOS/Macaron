#pragma once

#include <Devices/DeviceManager.hpp>
#include <Macaronlib/String.hpp>

#define ACKNOWLEDGE_DRIVER \
    DeviceManager::the().acknowledge_driver(this);

#define REGISTER_DRIVER(class_name) \
    class_name()                    \
    {                               \
        ACKNOWLEDGE_DRIVER          \
    }

namespace Kernel::Devices {

class Driver {
    friend class DeviceManager;

public:
    virtual String driver_name() = 0;
    virtual String driver_info() = 0;

protected:
    virtual void install() { }
};

}