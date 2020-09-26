#pragma once

#include "../Dirver.hpp"

namespace kernel::drivers {

class DiscDriver : public Driver {
public:
    virtual bool read(uint32_t, uint8_t, void*) { return false; }
    virtual bool write(uint32_t, uint8_t, void*) { return false; }
};

}