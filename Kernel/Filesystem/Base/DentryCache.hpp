#pragma once

#include <Macaronlib/HashTable.hpp>
#include <Macaronlib/List.hpp>
#include <Macaronlib/String.hpp>
#include <Macaronlib/Traits.hpp>

namespace Kernel::FileSystem {

class Dentry;
class VFS;
class Inode;

struct DentryLookuper {
    DentryLookuper(Dentry* parent, const String& name)
        : parent(parent)
        , name(name)
    {
    }

    Dentry* parent;
    const String& name;

    bool operator==(const DentryLookuper& other)
    {
        return parent == other.parent && name == other.name;
    }
};

class DentryCache {
    friend class Dentry;

    static constexpr auto max_capacity = 128;

public:
    DentryCache() = default;

    void init()
    {
        m_dentries = HashTable<Dentry>(max_capacity);
    }

    Dentry* lookup(Dentry* dentry, const String& name);
    Dentry* put(Dentry&& dentry);

private:
    bool ensure_capacity();

    bool is_in_lru(Dentry& dentry);
    void put_to_lru(Dentry& dentry);
    void remove_from_lru(Dentry& dentry);

private:
    HashTable<Dentry> m_dentries {};
    List<HashTable<Dentry>::Iterator> m_lru {};
};

extern DentryCache s_dentry_cache;

class Dentry {
    friend class DentryCache;
    friend class Inode;
    friend class VFS;

public:
    Dentry(Dentry* parent, const String& name, Inode* inode = nullptr)
        : m_parent(parent)
        , m_name(name)
        , m_inode(inode)
    {
    }

    int count() const { return m_count; }
    void update_count(int delta);

    Dentry* parent() const { return m_parent; }
    const String& name() const { return m_name; }
    Inode* inode() { return m_inode; }
    void set_inode(Inode* inode) { m_inode = inode; }
    void set_inode(Inode& inode) { m_inode = &inode; }

    Dentry* lookup(const String& name);

private:
    Dentry* m_parent;
    String m_name;
    Inode* m_inode;
    int m_count {};

    // DentryCache fields
    List<HashTable<Dentry>::Iterator>::Iterator m_lru_pos {};
    HashTable<Dentry>::Iterator m_ht_pos {};
};

}

using namespace Kernel::FileSystem;

struct DentryLookuperTraits : public Traits<DentryLookuperTraits> {
    static uint32_t hash(const DentryLookuper& dl)
    {
        return Traits<Dentry*>::hash(dl.parent) ^ Traits<String>::hash(dl.name);
    }
};

template <>
struct Traits<Dentry> : public GenericTraits<Dentry> {
    static uint32_t hash(const Dentry& dentry)
    {
        return Traits<Dentry*>::hash(dentry.parent()) ^ Traits<String>::hash(dentry.name());
    }

    static bool equals(const Dentry& dentry, const DentryLookuper& dl)
    {
        return dentry.parent() == dl.parent && dentry.name() == dl.name;
    }

    static bool equals(const Dentry& d1, const Dentry& d2)
    {
        return d1.parent() == d2.parent() && d1.name() == d2.name();
    }
};
