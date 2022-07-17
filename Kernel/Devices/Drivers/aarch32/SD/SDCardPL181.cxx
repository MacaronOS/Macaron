#include "SDCardPL181.hpp"
#include "MMCIPL181.hpp"

namespace Kernel::Devices {

SDCardPL181* SDCardPL181::construct(uint32_t id, uint32_t rca, MMCIPL181& mmci)
{
    if (mmci.set_block_size(rca, PL181_BLOCK_SIZE)) {
        return new SDCardPL181(id, rca, mmci);
    }
    return nullptr;
}

bool SDCardPL181::read_blocks(size_t block, size_t block_count, void* buffer)
{
    for (size_t i = 0; i < block_count; i++) {
        if (!m_mmci.read_block(m_rca, block + i, block_size(), (uint32_t*)((uintptr_t)buffer + i * block_size()))) {
            return false;
        }
    }
    return true;
}

bool SDCardPL181::write_blocks(size_t block, size_t block_count, void* buffer)
{
    for (size_t i = 0; i < block_count; i++) {
        return false;
    }
    return true;
}
}