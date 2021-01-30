#pragma once
#include "../monitor.hpp"
#include "../algo/String.hpp"

namespace kernel::tests {

#define FAILM(msg)                                  \
    {                                               \
        term_print(algorithms::String(msg) + "\n"); \
        return false;                               \
    }

#define FAIL()        \
    {                 \
        return false; \
    }

class Test {
public:
    Test(const algorithms::String& name)
        : m_name(name)
    {
    }

    virtual bool test()
    {
        return true;
    }
    
    algorithms::String name() const { return m_name; }

public:
    algorithms::String m_name;
};
}