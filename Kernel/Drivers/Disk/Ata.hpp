#pragma once

#include <Wisterialib/Common.hpp>
#include "DiskDriver.hpp"

#define WORDS_PER_SECTOR 256
#define BYTES_PER_SECTOR 512

namespace Kernel::Drivers {

enum class AdressingMode {
    Mode28,
    Mode48,
};

class Ata : public DiskDriver {
public:
    Ata(uint16_t port_base, bool master, DriverEntity driver_entity);
    bool install() override;
    bool read(uint32_t, uint8_t, void*) override;
    bool write(uint32_t, uint8_t, void*) override;

    bool read28(uint32_t, uint8_t, uint8_t*);
    bool write28(uint32_t, uint8_t, uint8_t*);

    bool flush();

private:
    uint32_t m_capacity {};
    bool m_master {};
    AdressingMode addressing_mode { AdressingMode::Mode28 };

    uint16_t m_data_port {};
    uint16_t m_error_port {};
    uint16_t m_sector_count_port {};
    uint16_t m_lba_low_port {};
    uint16_t m_lba_mid_port {};
    uint16_t m_lba_high_port {};
    uint16_t m_device_port {};
    uint16_t m_command_port {};
    uint16_t m_control_port {};

    static inline void handle_error(uint8_t);
};

}
