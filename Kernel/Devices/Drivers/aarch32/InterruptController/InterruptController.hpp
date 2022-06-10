#pragma once

#include <Macaronlib/Common.hpp>

namespace Kernel::Devices {

enum class InterruptType {
    LevelSensitive,
    EdgeTriggered,
};

class InterruptController {
public:
    virtual void enable_interrupt(uint32_t intno, InterruptType type) = 0;
    virtual uint32_t acknowledge_interrupt() = 0;
    virtual void end_of_interrupt(uint32_t intno) = 0;
};

}