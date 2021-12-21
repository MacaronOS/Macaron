#pragma once

#include "CharacterDevice.hpp"
#include "Driver.hpp"
#include <Filesystem/VFS/VFS.hpp>
#include <Libkernel/Logger.hpp>
#include <Macaronlib/Common.hpp>
#include <Macaronlib/String.hpp>

namespace Kernel::Drivers {

class CharacterDeviceDriver : public Driver, public CharacterDevice {
public:
    CharacterDeviceDriver(DriverEntity entity, const String& name)
        : Driver(entity, DriverType::CharacterDevice)
        , CharacterDevice(name)
    {
    }
};

}