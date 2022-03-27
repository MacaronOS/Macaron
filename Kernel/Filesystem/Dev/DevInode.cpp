
#include "DevInode.hpp"
#include "DevFileSystem.hpp"

namespace Kernel::FileSystem {

Inode* DevInode::create(const String& name, FileType, FilePermissions)
{
    auto inode = static_cast<DevInode*>(fs().allocate_inode());
    inode->m_name = name;
    m_children.push_back(inode);
    return inode;
}

}