#pragma once

#include "HashTable.hpp"
#include "Pair.hpp"
#include "Traits.hpp"

template <typename K, typename V, typename KTraits = Traits<K>>
class HashMap {
    using PairEntryType = Pair<K, V>;

    struct PairEntryTraits : public Traits<PairEntryType> {
        static uint32_t hash(const PairEntryType& entry) { return KTraits::hash(entry.first); }
        static bool equals(const PairEntryType& a, const PairEntryType& b) { return KTraits::equals(a.first, b.first); }
        static bool equals(const PairEntryType& entry, const K& key) { return KTraits::equals(entry.first, key); }
    };

    using HashTableType = HashTable<PairEntryType, PairEntryTraits>;

public:
    bool empty() const { return m_hash_table.empty(); }
    size_t size() const { return m_hash_table.size(); }
    size_t capacity() const { return m_hash_table.capacity(); }

    using Iterator = typename HashTableType::Iterator;
    using ConstIterator = typename HashTableType::ConstIterator;

    Iterator begin() { return m_hash_table.begin(); }
    ConstIterator begin() const { return m_hash_table.begin(); }

    Iterator end() { return m_hash_table.end(); }
    ConstIterator end() const { return m_hash_table.end(); }

    void clear() { m_hash_table.clear(); }

    bool contains(const K& key) const { return find(key) != end(); }

    void erase(const K& key)
    {
        auto it = find(key);
        if (it != end()) {
            erase(it);
        }
    }

    void erase(const Iterator& it) { m_hash_table.erase(it); }

    V& operator[](const K& key)
    {
        auto it = find(key);
        if (it != m_hash_table.end()) {
            return (*it).second;
        }
        return (*m_hash_table.insert({ key })).second;
    }

    Iterator find(const K& key) { return m_hash_table.template find<K, KTraits>(key); }
    ConstIterator find(const K& key) const { return m_hash_table.template find<K, KTraits>(key); }

private:
    HashTableType m_hash_table {};
};
