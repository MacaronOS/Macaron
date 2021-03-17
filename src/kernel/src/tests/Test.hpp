#pragma once
#include "../monitor.hpp"
#include <wisterialib/String.hpp>

namespace kernel::tests {

#define FAILM(msg)                                  \
    {                                               \
        term_print(String(msg) + "\n"); \
        return false;                               \
    }

#define FAIL()        \
    {                 \
        return false; \
    }

class Test {
public:
    Test(const String& name)
        : m_name(name)
    {
    }

    virtual bool test()
    {
        return true;
    }
    
    String name() const { return m_name; }

public:
    String m_name;
};
}