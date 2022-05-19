#pragma once

#include <Devices/Device.hpp>
#include <Devices/DeviceManager.hpp>
#include <Devices/Drivers/Driver.hpp>
#include <Libkernel/Assert.hpp>
#include <Macaronlib/String.hpp>

namespace Kernel::Devices {

/*
    DeviceDriver represents a special wrapper class that is used for installation of devices
    that are backed by a driver. It tries to install a driver with try_install and
    registers a device if the installation is successful.
*/

enum class DriverInstallationResult {
    Succeeded,
    Failed,
};

template <typename DriverKind, typename InstallableDevice>
class DeviceDriver : public DriverKind,
                     public Driver {
public:
    DeviceDriver(uint32_t major, uint32_t minor)
        : DriverKind(major, minor)
    {
    }

    virtual void install() override
    {
        auto installation_result = static_cast<InstallableDevice*>(this)->try_install();
        if (installation_result == DriverInstallationResult::Succeeded) {
            auto error = DeviceManager::the().register_device(this);
            if (error) {
                ASSERT_PANIC("[DeviceDriver] Error while registering an installed device driver");
            }
        }
    }
};

template <typename InstallableDevice>
using CharacterDeviceDriver = DeviceDriver<CharacterDevice, InstallableDevice>;

template <typename InstallableDevice>
using BlockDeviceDriver = DeviceDriver<BlockDevice, InstallableDevice>;

}
