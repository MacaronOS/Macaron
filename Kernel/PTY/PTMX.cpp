#include "PTMX.hpp"
#include "PTYMaster.hpp"
#include "PTYSlave.hpp"
#include <Filesystem/DevFS/DevFS.hpp>
#include <Libkernel/Logger.hpp>

namespace Kernel::Drivers {

void PTMX::open(FS::FileDescriptor& fd, FS::VNode& vnode)
{
    pts_count++;

    auto pty_slave = new PTYSlave(pts_count);
    auto pty_master = new PTYMaster();

    pty_slave->set_master(pty_master);
    pty_master->set_slave(pty_slave);

    auto devfs = static_cast<FS::DevFS*>(m_pts_directory.fs());
    auto pty_slave_node = devfs->create_device_node_inside_directory(m_pts_directory, pty_slave);
    auto pty_master_node = devfs->create_anonim_device_node(pty_master);

    fd.set_file(pty_master_node);
}

}