#pragma once

#include <wisterialib/Singleton.hpp>
#include <wisterialib/Vector.hpp>

#include "Test.hpp"

namespace kernel::tests {

class Tester {
public:
    Tester() = default;

    bool run_all();
    void add_test(Test* tests);

private:
    Vector<Test*> m_tests {};
};

}