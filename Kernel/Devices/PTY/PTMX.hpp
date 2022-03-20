#pragma once

#include "../Device.hpp"
#include <Filesystem/DevFS/DevFSNode.hpp>

namespace Kernel::Drivers {

// PTMX - a PTY multiplexer
class PTMX : public Device {
public:
    PTMX(FS::DevFSNode& pts_directory)
        : Device(5, 2, DeviceType::Character)
        , m_pts_directory(pts_directory)
    {
    }

    void open(FS::FileDescriptor& fd, FS::VNode& vnode) override;

private:
    FS::DevFSNode& m_pts_directory;
    size_t pts_count {};
};

}