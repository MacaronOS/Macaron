#pragma once

#include "Device.hpp"

namespace Kernel::Devices {

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

    bool register_device(Device* device)
    {
        if (device->install()) {
            m_devices[device->major()][device->minor()][static_cast<uint8_t>(device->type())] = device;
            return true;
        }
        return false;
    }

    bool register_device(Device& device)
    {
        return register_device(&device);
    }

    Device* get_device(uint32_t major, uint32_t minor, DeviceType type)
    {
        return m_devices[major][minor][static_cast<uint8_t>(type)];
    }

    friend class Iterator;
    Iterator begin() { return Iterator(0, *this); }
    Iterator end() { return Iterator(majors * minors * types, *this); }

    void register_initial_devices();

private:
    Device* m_devices[majors][minors][types] {};
};

}