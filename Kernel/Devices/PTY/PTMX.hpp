#pragma once

#include "../Device.hpp"

namespace Kernel::Devices {

using namespace FileSystem;

// PTMX - a PTY multiplexer
class PTMX : public Device {
public:
    PTMX()
        : Device(5, 0, DeviceType::Char)
    {
    }
    
    virtual void open(FileSystem::FileDescription& fd) override;

private:
    size_t pts_count {};
};

extern PTMX ptmx;

}