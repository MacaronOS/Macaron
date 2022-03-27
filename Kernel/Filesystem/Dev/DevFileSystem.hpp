#pragma once

#include <Devices/DeviceManager.hpp>
#include <FileSystem/Base/FileSystem.hpp>

namespace Kernel::FileSystem {

class DevFileSystem : public FileSystem {
public:
    virtual void init() override;
    virtual Inode* allocate_inode() override;

private:
    size_t m_nodes_cnt {};
};

}