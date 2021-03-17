#pragma once

#include "PCI.hpp"

#include <wisterialib/Vector.hpp>
#include <drivers/base/Driver.hpp>
#include <wisterialib/common.hpp>

namespace kernel::drivers {

class PCIDevice {
    friend class PCI;

    enum class BaseAdressRegisterType {
        MemoryMapping = 0,
        IO = 1,
    };

    class BaseAdressRegister {
    public:
        bool prefetchable;
        uint8_t* address;
        BaseAdressRegisterType type;
    };

private:
    // can be instantiated only in PCI
    PCIDevice(uint16_t bus, uint16_t device, uint16_t function, uint16_t vendor_id = 0);

public:
    PCIDevice() = delete;

    uint32_t read_base_register(uint8_t bar);
    uint32_t read(uint16_t regoff);

    uint16_t vendor_id() const { return m_vendor_id; }
    uint16_t device_id() const { return m_device_id; }

private:
    uint32_t m_port_base {};
    uint32_t m_interrupt {};

    uint16_t m_bus {};
    uint16_t m_device {};
    uint16_t m_function {};

    uint16_t m_vendor_id {};
    uint16_t m_device_id {};

    uint8_t m_class_id {};
    uint8_t m_subclass_id {};
    uint8_t m_interface_id {};

    uint8_t m_revision {};

    BaseAdressRegister m_base_adress_registers[6] {};
};
}