#pragma once

#include "DentryCache.hpp"
#include "FileDescriptor.hpp"

#include <Libkernel/Assert.hpp>
#include <Libkernel/Graphics/VgaTUI.hpp>
#include <Tasking/Net/LocalSocket.hpp>

#include <Macaronlib/Common.hpp>
#include <Macaronlib/Runtime.hpp>
#include <Macaronlib/String.hpp>
#include <Macaronlib/Vector.hpp>

namespace Kernel::FS {

enum class FileType {
    FIFO = 0x1000,
    CharDevice = 0x2000,
    Directory = 0x4000,
    BlockDevice = 0x6000,
    File = 0x8000,
    Symlink = 0xA000,
    Socket = 0xC000,
    Mountpoint,
    NOTAFILE,
};

enum class FilePermission {
    Execute = 0x001,
    Write = 0x002,
    Read = 0x004,
    GroupExecute = 0x008,
    GroupWrite = 0x010,
    GroupRead = 0x020,
    UserExecute = 0x040,
    UserWrite = 0x080,
    UserRead = 0x100
};

typedef uint16_t FilePermissions;

class FS;
class Dentry;

// Represents an object that allows different filesystems communicate which each other.
class VNode {

public:
    class Mountpoint {

    public:
        Mountpoint(const String& name, VNode* vnode);
        Mountpoint(const Mountpoint& mp);
        Mountpoint(Mountpoint&& mp);
        Mountpoint& operator=(const Mountpoint& mp);
        Mountpoint& operator=(Mountpoint&& mp);

        String& name() { return m_name; }
        VNode& vnode() { return *m_vnode; }

    private:
        String m_name;
        VNode* m_vnode;
    };

public:
    VNode() = default;
    VNode(FS* fs, uint32_t inode);
    virtual ~VNode() = default;

    virtual void read(void* buffer, size_t size, FileDescriptor& fd);
    virtual void write(void* buffer, size_t size, FileDescriptor&);
    virtual void truncate(size_t size) { }
    virtual void open(FileDescriptor&) { }
    virtual void close(FileDescriptor&) { }
    virtual void create(const String& name, FileType, FilePermissions) { }
    virtual void mmap(void* addr, size_t size) { }
    virtual void ioctl(uint32_t request) { }
    virtual bool can_read(FileDescriptor&) { return false; }
    void lookup(Dentry& dentry);

    void mount(Mountpoint& mountpoint);
    void mount(Mountpoint&& mountpoint);

    virtual uint32_t size() const { return 0; }
    Vector<Mountpoint>& mounted_dirs() { return m_mountpoints; }

    // VNode System funcs
    FS* fs() const { return m_fs; }
    uint32_t vnode() const { return m_vnode; }
    size_t ref_count() const { return m_ref_count; }
    void inc_ref_count() { m_ref_count++; }

    // Socket funcs
    net::LocalSocket* socket() { return m_socket; }
    void bind_socket() { m_socket = new net::LocalSocket(); }

protected:
    virtual void lookup_derived(Dentry& dentry) { }

protected:
    FS* m_fs { nullptr };
    uint32_t m_vnode { 0 };
    Vector<Mountpoint> m_mountpoints {};
    bool m_dirty { false };
    size_t m_ref_count { 0 };

    net::LocalSocket* m_socket { nullptr };
};

class VNodeStorage {
public:
    VNodeStorage() = default;

    VNode* find(FS* fs, uint32_t vnode);
    void push(VNode* vnode);
    void push(VNode& vnode);

private:
    Vector<VNode*> vnodes {};
};

}