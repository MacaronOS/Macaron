#pragma once
#include "common.hpp"
#include "extras.hpp"

template <typename T>
class ListNode {
public:
    ListNode() = default;
    ~ListNode() = default;

    explicit ListNode(const T& val)
        : m_val(val)
    {
    }
    explicit ListNode(T&& val)
        : m_val(move(val))
    {
    }

    ListNode<T>* next() const { return m_next; }
    ListNode<T>* prev() const { return m_prev; }

    void set_next(ListNode<T>* next) { m_next = next; }
    void set_prev(ListNode<T>* prev) { m_prev = prev; }

public:
    T m_val;

private:
    ListNode<T>* m_next {};
    ListNode<T>* m_prev {};
};

template <typename T>
class List {
public:
    using ValueType = T;

    List() { tie_tails(); }
    ~List() { clear(); }
    List(const List& list);
    List& operator=(const List& list);

    void push_front(const T& val) { push_front(new ListNode<ValueType>(val)); }
    void push_front(T&& val) { push_front(new ListNode<ValueType>(move(val))); }
    void push_back(const T& val) { push_back(new ListNode<ValueType>(val)); }
    void push_back(T&& val) { push_back(new ListNode<ValueType>(move(val))); }

    void clear();
    uint32_t size() const { return m_size; }

    template <class NodeType>
    class Iterator {
        friend class List<ValueType>;

    public:
        explicit Iterator(NodeType* node_ptr = nullptr)
            : m_node_ptr(node_ptr)
        {
        }

        ValueType& operator*() const { return m_node_ptr->m_val; }
        ValueType operator*() { return m_node_ptr->m_val; }

        ValueType* operator->() { return &m_node_ptr->m_val; }
        ValueType const* operator->() const { return &m_node_ptr->m_val; }

        bool operator==(const Iterator& it) const { return m_node_ptr == it.m_node_ptr; }
        bool operator!=(const Iterator& it) const { return m_node_ptr != it.m_node_ptr; }

        Iterator operator++()
        {
            m_node_ptr = m_node_ptr->next();
            return *this;
        }
        Iterator operator--()
        {
            m_node_ptr = m_node_ptr->prev();
            return *this;
        }

        Iterator operator++(int)
        {
            auto cp = *this;
            m_node_ptr = m_node_ptr->next();
            return cp;
        }
        Iterator operator--(int)
        {
            auto cp = *this;
            m_node_ptr = m_node_ptr->prev();
            return cp;
        }

    protected:
        NodeType* m_node_ptr {};
    };

    using DefaultIterator = Iterator<ListNode<ValueType>>;
    using ConstIterator = Iterator<const ListNode<ValueType>>;

    ConstIterator begin() const { return ConstIterator(m_head.next()); }
    ConstIterator rbegin() const { return ConstIterator(m_tail.prev()); }
    ConstIterator end() const { return ConstIterator(&m_tail); }
    ConstIterator rend() const { return ConstIterator(&m_head); }

    DefaultIterator begin() { return DefaultIterator(m_head.next()); }
    DefaultIterator rbegin() { return DefaultIterator(m_tail.prev()); }
    DefaultIterator end() { return DefaultIterator(&m_tail); }
    DefaultIterator rend() { return DefaultIterator(&m_head); }

    DefaultIterator find(const ValueType& value);
    ConstIterator find(const ValueType& value) const;

    DefaultIterator remove(const DefaultIterator& del_it);

public:
    void tie_tails();
    void push_front(ListNode<ValueType>* node);
    void push_back(ListNode<ValueType>* node);

public:
    ListNode<ValueType> m_head {};
    ListNode<ValueType> m_tail {};

    uint32_t m_size {};
};

template <typename T>
void List<T>::push_front(ListNode<ValueType>* node)
{
    node->set_prev(&m_head);
    m_head.next()->set_prev(node);
    node->set_next(m_head.next());
    m_head.set_next(node);
    m_size++;
}

template <typename T>
void List<T>::push_back(ListNode<ValueType>* node)
{
    node->set_next(&m_tail);
    m_tail.prev()->set_next(node);
    node->set_prev(m_tail.prev());
    m_tail.set_prev(node);
    m_size++;
}

template <typename T>
void List<T>::clear()
{
    auto cur_node = m_head.next();

    while (cur_node != &m_tail) {
        auto next_node = cur_node->next();
        delete cur_node;
        cur_node = next_node;
    }

    tie_tails();
}

template <typename T>
void List<T>::tie_tails()
{
    m_head.set_next(&m_tail);
    m_tail.set_prev(&m_head);
}

template <typename T>
typename List<T>::ConstIterator List<T>::find(const ValueType& value) const
{
    for (auto it = this->begin(); it != this->end(); ++it) {
        if (*it == value) {
            return it;
        }
    }

    return end();
}

template <typename T>
typename List<T>::DefaultIterator List<T>::find(const ValueType& value)
{
    for (auto it = this->begin(); it != this->end(); ++it) {
        if (*it == value) {
            return it;
        }
    }

    return end();
}

template <typename T>
typename List<T>::DefaultIterator List<T>::remove(const List::DefaultIterator& del_it)
{
    auto node_ptr = del_it.m_node_ptr;
    if (node_ptr->prev()) {
        node_ptr->prev()->set_next(node_ptr->next());
    }
    if (node_ptr->next()) {
        node_ptr->next()->set_prev(node_ptr->prev());
    }
    auto prev = del_it;
    --prev;
    delete node_ptr;
    return prev;
}

template <typename T>
List<T>::List(const List& list)
{
    tie_tails();
    for (const auto& el : list) {
        push_back(el);
    }
}

template <typename T>
List<T>& List<T>::operator=(const List& list)
{
    clear();
    for (const auto& el : list) {
        push_back(el);
    }
    return *this;
}
