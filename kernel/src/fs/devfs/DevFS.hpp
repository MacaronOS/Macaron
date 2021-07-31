#pragma once

#include "DevFSNode.hpp"

#include <fs/base/VNode.hpp>
#include <fs/base/fs.hpp>

namespace kernel::fs::devfs {

class DevFS : public FS {
public:
    DevFS(VNodeStorage& vnode_storage);
    bool init();

    VNode& root() override { return m_root; };
    VNode* finddir(VNode& parent, const String& devname) override;
    Vector<String> listdir(VNode& directory) override;
    uint32_t read(VNode& file, uint32_t offset, uint32_t size, void* buffer) override;
    bool mmap(VNode& file, uint32_t addr, uint32_t size) override;
    bool ioctl(VNode& file, uint32_t request) override;
    bool can_read(VNode& vnode, uint32_t offset) override;

private:
    static DevFSNode& ToDevFSNode(VNode& device) { return reinterpret_cast<DevFSNode&>(device); }
    static DevFSNode* ToDevFSNode(VNode* device) { return reinterpret_cast<DevFSNode*>(device); }

private:
    DevFSNode m_root;
    VNodeStorage& m_vnode_storage;
    uint32_t devnodes {};
};

}