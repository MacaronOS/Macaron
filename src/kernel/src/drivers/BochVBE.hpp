#pragma once

#include "base/CharacterDevice.hpp"
#include "base/Driver.hpp"
#include "base/DriverEntity.hpp"

#include <drivers/pci/PCIDevice.hpp>

#include <types.hpp>

namespace kernel::drivers {

class BochVBE : public CharacterDevice {
public:
    BochVBE(PCIDevice* pci_device);
    bool install() override;
    bool mmap(uint32_t addr, uint32_t size) override;

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
    inline static void write(IndexRegister reg, uint16_t data);
    inline static uint16_t read(IndexRegister reg);

private:
    PCIDevice* m_pci_device;
    uint32_t* m_pixels { nullptr };
    uint32_t m_pixels_length { 0 };
};
}