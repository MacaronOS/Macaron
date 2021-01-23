#pragma once

#include "../algo/Singleton.hpp"
#include "../algo/Vector.hpp"

#include "Test.hpp"

namespace kernel::tests {

class Tester {
public:
    Tester() = default;

    bool run_all();
    void add_test(Test* tests);

private:
    algorithms::Vector<Test*> m_tests {};
};

}