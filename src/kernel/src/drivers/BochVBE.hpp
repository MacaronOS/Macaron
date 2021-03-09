#pragma once

#include "base/CharacterDevice.hpp"
#include "base/Driver.hpp"
#include "base/DriverEntity.hpp"

#include <drivers/pci/PCIDeviceDriver.hpp>

#include <types.hpp>

namespace kernel::drivers {

class BochVBE : public PCIDeviceDriver, public CharacterDevice {
public:
    BochVBE(PCIDevice* pci_device);
    bool install() override;

private:
    enum class IndexRegister : uint16_t {
        ID,
        XRes,
        YRes,
        BPP,
        Enable,
        Bank,
        VirtWidth,
        VirtHeight,
        XOffset,
        YOffset,
    };
    inline void write(IndexRegister reg, uint16_t data);
    inline uint16_t read(IndexRegister reg);
};
}