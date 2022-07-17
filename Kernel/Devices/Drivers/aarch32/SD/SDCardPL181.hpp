#pragma once

#include <Devices/Device.hpp>

namespace Kernel::Devices {

class MMCIPL181;

class SDCardPL181 : public BlockDevice {
    friend class MMCIPL181;
    static constexpr size_t PL181_BLOCK_SIZE = 512;

public:
    SDCardPL181(uint32_t id, uint32_t rca, MMCIPL181& mmci)
        : BlockDevice(3, id)
        , m_rca(rca)
        , m_mmci(mmci)
    {
    }

    virtual size_t block_size() override
    {
        return PL181_BLOCK_SIZE;
    }

    virtual bool read_blocks(size_t block, size_t block_count, void* buffer) override;
    virtual bool write_blocks(size_t block, size_t block_count, void* buffer) override;

    // ^MMCIPL181
    static SDCardPL181* construct(uint32_t id, uint32_t rca, MMCIPL181& mmci);

private:
    uint32_t m_rca;
    MMCIPL181& m_mmci;
};

}