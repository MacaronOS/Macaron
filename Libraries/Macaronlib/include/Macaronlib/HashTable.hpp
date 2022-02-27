#pragma once

#include "Common.hpp"
#include "HashFunctions.hpp"
#include "Memory.hpp"
#include "Traits.hpp"

template <typename HashTableType, typename ValueType, typename BucketType>
class HashTableIterator {
    friend HashTableType;

public:
    HashTableIterator() = default;

    ValueType& operator*() { return *m_cur_bucket->slot(); }
    ValueType* operator->() { return m_cur_bucket->slot(); }

    bool operator==(const HashTableIterator& other) const { return m_cur_bucket == other.m_cur_bucket; }
    bool operator!=(const HashTableIterator& other) const { return m_cur_bucket != other.m_cur_bucket; }

    HashTableIterator operator++()
    {
        skip_to_next();
        return *this;
    }

    HashTableIterator operator++(int)
    {
        auto cp = *this;
        skip_to_next();
        return cp;
    }

private:
    HashTableIterator(BucketType* cur_bucket, BucketType* end_bucket)
        : m_cur_bucket(cur_bucket)
        , m_end_bucket(end_bucket)
    {
        if (m_cur_bucket && !m_cur_bucket->used()) {
            skip_to_next();
        }
    }

    void skip_to_next()
    {
        if (!m_cur_bucket) {
            return;
        }
        do {
            m_cur_bucket++;
        } while (m_cur_bucket < m_end_bucket && !m_cur_bucket->used() && !m_cur_bucket->deleted());

        if (m_cur_bucket >= m_end_bucket) {
            m_cur_bucket = nullptr;
        }
    }

private:
    BucketType* m_cur_bucket {};
    BucketType* m_end_bucket {};
};

template <typename T, typename TypeTraits = Traits<T>>
class HashTable {
    static constexpr auto load_factor = 60;

    struct Bucket {
        static constexpr auto used_flag = 1 << 1;
        static constexpr auto deleted_flag = 1 << 2;

        uint8_t flags {};

        bool used() const { return flags & used_flag; }
        void set_used() { flags |= used_flag; }
        void unset_used() { flags &= ~used_flag; }

        bool deleted() const { return flags & deleted_flag; }
        void set_deleted() { flags |= deleted_flag; }
        void unset_deleted() { flags &= ~deleted_flag; }

        alignas(T) uint8_t storage[sizeof(T)];
        T* slot() { return reinterpret_cast<T*>(storage); }
        const T* slot() const { return reinterpret_cast<const T*>(storage); }
    };

public:
    HashTable() = default;
    explicit HashTable(size_t capacity) { rehash(capacity); }
    ~HashTable() { clear(); }

    HashTable(const HashTable& other)
    {
        *this = other;
    }

    HashTable& operator=(const HashTable& other)
    {
        if (this != &other) {
            clear();
            rehash(other.size());
            for (auto& el : other) {
                insert(el);
            }
        }
        return *this;
    }

    HashTable(HashTable&& other)
    {
        *this = move(other);
    }

    HashTable& operator=(HashTable&& other)
    {
        if (this != &other) {
            clear();
            m_size = other.m_size;
            m_capacity = other.m_capacity;
            m_buckets = other.m_buckets;
            other.m_size = 0;
            other.m_capacity = 0;
            other.m_buckets = 0;
        }
        return *this;
    }

    bool empty() const { return !m_size; }
    size_t size() const { return m_size; }
    size_t capacity() const { return m_capacity; }

    using Iterator = HashTableIterator<HashTable, T, Bucket>;
    using ConstIterator = HashTableIterator<const HashTable, const T, const Bucket>;

    Iterator begin() { return Iterator(m_buckets, m_buckets + m_capacity); }
    ConstIterator begin() const { return ConstIterator(m_buckets, m_buckets + m_capacity); }

    Iterator end() { return Iterator(nullptr, nullptr); }
    ConstIterator end() const { return ConstIterator(nullptr, nullptr); }

    void reserve(size_t size)
    {
        if (m_size >= size) {
            return;
        }
        rehash((size * (100 + load_factor) + 99) / 100);
    }

    void clear()
    {
        if (!m_buckets) {
            return;
        }

        for (size_t i = 0; i < m_capacity; i++) {
            if (m_buckets[i].used()) {
                m_buckets[i].slot()->~T();
            }
        }

        free(m_buckets);
        m_buckets = nullptr;
        m_capacity = 0;
        m_size = 0;
    }

    Iterator insert(T&& val)
    {
        auto& bucket = lookup_for_writing(val);
        new (bucket.slot()) T(move(val));
        bucket.set_used();
        bucket.unset_deleted();
        return Iterator(&bucket, m_buckets + m_capacity);
    }

    Iterator insert(const T& val)
    {
        return insert(T(val));
    }

    bool contains(const T& val) const
    {
        return find(val) != end();
    }

    void erase(const T& val)
    {
        auto it = find(val);
        if (it != end()) {
            erase(it);
        }
    }

    void erase(const Iterator& it)
    {
        auto& bucket = *it.m_cur_bucket;
        bucket.slot()->~T();
        bucket.set_deleted();
        bucket.unset_used();
        m_size--;
    }

    template <typename SearchType = T, typename SearchTraits = TypeTraits>
    Iterator find(const SearchType& val) { return Iterator(lookup_for_reading<SearchType, SearchTraits>(val), m_buckets + m_capacity); }

    template <typename SearchType = T, typename SearchTraits = TypeTraits>
    ConstIterator find(const SearchType& val) const { return ConstIterator(lookup_for_reading<SearchType, SearchTraits>(val), m_buckets + m_capacity); }

private:
    template <typename SearchType, typename SearchTraits>
    Bucket* lookup_for_reading(const SearchType& val) const
    {
        Bucket* bucket_for_val = nullptr;

        auto hash = SearchTraits::hash(val);
        for (size_t i = 0; i < m_capacity; i++) {
            auto& bucket = m_buckets[(hash + i) % m_capacity];
            if (bucket.deleted()) {
                continue;
            }
            if (!bucket.used()) {
                break;
            }
            if (TypeTraits::equals(*bucket.slot(), val)) {
                bucket_for_val = &bucket;
                break;
            }
        }

        return bucket_for_val;
    }

    Bucket& lookup_for_writing(const T& val)
    {
        // Check if element is already presented
        auto try_value_exists_bucket = lookup_for_reading<T, TypeTraits>(val);
        if (try_value_exists_bucket) {
            return *try_value_exists_bucket;
        }

        // If element isn't presented, it should be added
        if (need_rehash()) {
            rehash(m_capacity * 2 + 4);
        }

        m_size++;

        auto hash = TypeTraits::hash(val);
        for (size_t i = 0; i < m_capacity; i++) {
            auto& bucket = m_buckets[(hash + i) % m_capacity];
            if (bucket.deleted() || !bucket.used()) {
                return bucket;
            }
        }
    }

    bool need_rehash() const { return (m_size + 1) * 100 >= m_capacity * load_factor; }

    void rehash(size_t capacity)
    {
        auto old_capacity = m_capacity;
        auto old_buckets = m_buckets;

        m_size = 0;
        m_capacity = capacity;
        m_buckets = (Bucket*)malloc(sizeof(Bucket) * m_capacity);
        memset(m_buckets, 0, sizeof(Bucket) * m_capacity);

        for (size_t i = 0; i < old_capacity; i++) {
            auto& old_bucket = old_buckets[i];
            if (old_bucket.used() && !old_bucket.deleted()) {
                insert(move(*old_bucket.slot()));
                old_bucket.slot()->~T();
            }
        }

        free(old_buckets);
    }

private:
    Bucket* m_buckets {};
    size_t m_size {};
    size_t m_capacity {};
};