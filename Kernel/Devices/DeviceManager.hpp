#pragma once

#include <Devices/Device.hpp>
#include <Libkernel/KError.hpp>
#include <Libkernel/Logger.hpp>
#include <Macaronlib/HashMap.hpp>
#include <Macaronlib/String.hpp>

namespace Kernel::Devices {

class Driver;

class DeviceManager {
public:
    static constexpr auto majors = 64;
    static constexpr auto minors = 64;
    static constexpr auto types = 2;

    class Iterator {
        friend class DeviceManager;

    public:
        Iterator operator++()
        {
            skip_to_valid_device();
            return *this;
        }

        Device& operator*() { return *get_current_device(); }
        Device* operator->() { return get_current_device(); }

        bool operator==(const Iterator& other) const { return m_device_index == other.m_device_index; }
        bool operator!=(const Iterator& other) const { return m_device_index != other.m_device_index; }

    private:
        Iterator(size_t device_index, DeviceManager& device_manager)
            : m_device_index(device_index)
            , m_device_manager(device_manager)
        {
            if (m_device_index < majors * minors * types && !get_current_device()) {
                skip_to_valid_device();
            }
        }

        void skip_to_valid_device()
        {
            for (m_device_index++; m_device_index < majors * minors * types; m_device_index++) {
                if (get_current_device()) {
                    break;
                }
            }
        }

        Device* get_current_device()
        {
            auto type = m_device_index % types;
            auto minor = m_device_index / types % minors;
            auto major = m_device_index / types / minors;
            return m_device_manager.get_device(major, minor, static_cast<DeviceType>(type));
        }

    private:
        size_t m_device_index;
        DeviceManager& m_device_manager;
    };

    static DeviceManager& the()
    {
        static DeviceManager the;
        return the;
    }

    KError register_device(Device* device)
    {
        if (!device) {
            return KError(ENODEV);
        }

        auto major = device->major();
        auto minor = device->minor();
        auto type = static_cast<uint8_t>(device->type());

        auto& device_slot = m_devices[major][minor][type];

        // Alllow rewrites to the device slot untill we does not support a device tree.
        // TOOD: prioritize drivers.
        // if (device_slot) {
        // return KError(EEXIST);
        // }

        device_slot = device;
        return KError(0);
    }

    bool register_device(Device& device)
    {
        return register_device(&device);
    }

    Device* get_device(uint32_t major, uint32_t minor, DeviceType type)
    {
        return m_devices[major][minor][static_cast<uint8_t>(type)];
    }

    BlockDevice* get_block_device(uint32_t major, uint32_t minor)
    {
        return static_cast<BlockDevice*>(get_device(major, minor, DeviceType::Block));
    }

    Driver* find_attached_driver_by_name(const String& name);
    void register_virtual_devices();
    void acknowledge_driver(Driver* driver);
    void install_acknowledged_drivers();

    friend class Iterator;
    Iterator begin() { return Iterator(0, *this); }
    Iterator end() { return Iterator(majors * minors * types, *this); }

private:
    Device* m_devices[majors][minors][types] {};
    HashMap<String, Driver*> m_acknowledged_drivers {};
    HashMap<String, Driver*> m_attached_drivers {};
};

}