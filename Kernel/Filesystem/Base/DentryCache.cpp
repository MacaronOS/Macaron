#include "DentryCache.hpp"
#include "Inode.hpp"

namespace Kernel::FileSystem {

DentryCache s_dentry_cache;

Dentry* DentryCache::lookup(Dentry* parent, const String& name)
{
    auto dentry_it = m_dentries.find<DentryLookuper, DentryLookuperTraits>(DentryLookuper(parent, name));
    if (dentry_it == m_dentries.end()) {
        return nullptr;
    }
    return &*dentry_it;
}

Dentry* DentryCache::put(Dentry&& dentry)
{
    if (!ensure_capacity()) {
        return nullptr;
    }

    auto new_dentry_it = m_dentries.insert(move(dentry));
    (*new_dentry_it).m_ht_pos = new_dentry_it;

    return &*new_dentry_it;
}

bool DentryCache::ensure_capacity()
{
    if (m_dentries.size() == max_capacity) {
        if (m_lru.empty()) {
            return false;
        }

        auto most_unused_dentry = m_lru.rbegin();
        m_dentries.erase((*most_unused_dentry)->m_ht_pos);
        m_lru.remove(most_unused_dentry);
    }

    return true;
}

bool DentryCache::is_in_lru(Dentry& dentry)
{
    return dentry.m_lru_pos != m_lru.end();
}

void DentryCache::put_to_lru(Dentry& dentry)
{
    m_lru.push_front(dentry.m_ht_pos);
    dentry.m_lru_pos = m_lru.begin();
}

void DentryCache::remove_from_lru(Dentry& dentry)
{
    m_lru.remove(dentry.m_lru_pos);
    dentry.m_lru_pos = m_lru.end();
}

void Dentry::update_count(int delta)
{
    m_count += delta;

    if (m_count <= 0 && !s_dentry_cache.is_in_lru(*this)) {
        s_dentry_cache.put_to_lru(*this);
    }

    if (m_count == 1 && s_dentry_cache.is_in_lru(*this)) {
        s_dentry_cache.remove_from_lru(*this);
    }

    if (parent()) {
        parent()->update_count(delta);
    }
}

Dentry* Dentry::lookup(const String& name)
{
    auto dentry = s_dentry_cache.lookup(this, name);
    if (!dentry) {
        Dentry new_denry(this, name);
        if (m_inode) {
            m_inode->lookup(new_denry);
        }
        dentry = s_dentry_cache.put(move(new_denry));
    }

    m_count++;
    if (m_count == 1 && s_dentry_cache.is_in_lru(*this)) {
        s_dentry_cache.remove_from_lru(*this);
    }

    return dentry;
}

}