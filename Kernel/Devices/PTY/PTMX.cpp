#include "PTMX.hpp"
#include "PTYMaster.hpp"
#include "PTYSlave.hpp"
#include <FileSystem/Dev/DevInode.hpp>
#include <FileSystem/VFS/VFS.hpp>
#include <Libkernel/Logger.hpp>

namespace Kernel::Devices {

void PTMX::open(FileSystem::FileDescription& fd)
{
    pts_count++;

    auto pty_slave = new PTYSlave(pts_count);
    auto pty_master = new PTYMaster();

    pty_slave->set_master(pty_master);
    pty_master->set_slave(pty_slave);

    auto pts_directory = static_cast<DevInode*>(VFS::the().resolve_path("/dev/pts"));
    if (!pts_directory) {
        return;
    }

    auto pty_slave_node = static_cast<DevInode*>(pts_directory->create(pty_slave->name(), FileType::CharDevice, 1));
    if (!pty_slave_node) {
        return;
    }

    pty_slave_node->m_device = pty_slave;

    fd.file = pty_master;
}

}