#pragma once

#include "Layout.hpp"

namespace Kernel::Memory {

enum Flags {
    Present = 1 << 0,
    Write = 1 << 1,
    User = 1 << 2,
};

struct Region {
    // TODO: kernel will need multiple regions for large memory ranges (f.e. sockets)
    // It should be a Vector of Regions
    static Region KernelRegion()
    {
        auto start_page = HIGHER_HALF_OFFSET / 4096;
        auto start_frame = 0;
        auto end_page = 0xc0800000 / 4096;

        Region reg {};
        reg.page = start_page;
        reg.frame = start_frame;
        reg.pages = end_page - start_page;
        reg.flags = 0x7;

        return reg;
    }

    bool includes(uint32_t _page) const { return _page >= page && _page <= page + pages; }

    enum class Type {
        Allocated, // memory, allocated from the PMM
        Mapping, // memory which could potentially be reserved and used for device communication
    };
    enum class Mapping {
        Shared,
        Anonimous,
    };

    Type type;
    Mapping mapping; // set, if type is Mapping

    uint32_t page; // starting page
    uint32_t frame; // starting frame. set if type is mapping
    uint32_t pages; // size in pages

    uint32_t page_dir_phys;

    uint32_t flags;
};

}