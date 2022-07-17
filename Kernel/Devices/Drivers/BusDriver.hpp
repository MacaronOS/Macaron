#pragma once

#include <Devices/Device.hpp>
#include <Devices/DeviceManager.hpp>
#include <Devices/Drivers/Driver.hpp>

namespace Kernel::Devices {

class BusDriver : public Driver {
protected:
    template <typename DeviceType, typename... Args>
    void register_device(Args&&... args)
    {
        if constexpr (!is_base_of<Device, DeviceType>) {
            static_assert("[BusDriver]: registered device should be derived from Device");
        }
        auto device = DeviceType::construct(forward<Args>(args)...);
        if (device) {
            DeviceManager::the().register_device(device);
        }
    }
};

}