#pragma once

#include <Macaronlib/Common.hpp>

namespace Kernel::FS {

class VNode;
class FileDescriptor {
public:
    FileDescriptor() = default;

    int flags() const { return m_flags; }
    size_t offset() const { return m_offset; }

    void set_flags(int flags) { m_flags |= flags; }

    void inc_offset(size_t offset) { m_offset += offset; }
    void dec_offset(size_t offset) { m_offset -= offset; }
    void set_offset(size_t offset) { m_offset = offset; }

    void set_file(VNode* vnode) { m_vnode = vnode; }
    void set_file(VNode& vnode) { m_vnode = &vnode; }
    VNode* vnode() { return m_vnode; }

private:
    VNode* m_vnode { nullptr };
    size_t m_offset { 0 };
    int m_flags { 0 };
};

}