#pragma once

#include <Devices/Drivers/DeviceDriver.hpp>

namespace Kernel::Devices {

enum class AtaAdressingMode {
    Mode28,
    Mode48,
};

class ATA : public BlockDeviceDriver<ATA> {
public:
    ATA(uint16_t port_base, bool master);

    // ^DeviceDriver
    DriverInstallationResult try_install();

    // ^Driver
    virtual String driver_name() override
    {
        return "ATA";
    }
    virtual String driver_info() override
    {
        return "MacaronOS x86 ATA driver";
    }

    // ^BlockDevice
    virtual size_t block_size() override;
    virtual bool read_blocks(size_t block, size_t block_count, void* buffer) override;
    virtual bool write_blocks(size_t block, size_t block_count, void* buffer) override;

private:
    // Read/Write in Mode28.
    bool read28(uint32_t lba, uint8_t sector_count, uint8_t* addr);
    bool write28(uint32_t lba, uint8_t sector_count, uint8_t* addr);
    // Gives the drive a 400ns delay to reset
    // DRQ and set BSY bits.
    void make_400ns_delay();
    // Waits until device has an apropiate value,
    // also checks an error bit and processes it if an error occurs.
    bool wait_bit(uint8_t bit, uint8_t val);
    bool flush();

private:
    bool m_master {};
    uint16_t m_data_port {};
    uint16_t m_error_port {};
    uint16_t m_sector_count_port {};
    uint16_t m_lba_low_port {};
    uint16_t m_lba_mid_port {};
    uint16_t m_lba_high_port {};
    uint16_t m_device_port {};
    uint16_t m_command_port {};
    uint16_t m_control_port {};
    size_t m_capacity {};
    AtaAdressingMode m_addressing_mode { AtaAdressingMode::Mode28 };
};

}