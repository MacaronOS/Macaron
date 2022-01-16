#pragma once

#include <Drivers/Base/CharacterDevice.hpp>
#include <Filesystem/DevFS/DevFSNode.hpp>

namespace Kernel::Drivers {

// PTMX - a PTY multiplexer
class PTMX : public CharacterDevice {
public:
    PTMX(FS::DevFSNode& pts_directory)
        : CharacterDevice("ptmx")
        , m_pts_directory(pts_directory)
    {
    }

    void open(FS::FileDescriptor& fd, FS::VNode& vnode) override;

private:
    FS::DevFSNode& m_pts_directory;
    size_t pts_count {};
};

}