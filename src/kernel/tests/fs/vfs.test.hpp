#pragma once
#include "../Test.hpp"

namespace kernel::tests {

class TestVFS : public Test {
public:
    TestVFS()
        : Test("TestVFS")
    {
    }
    bool test() override;
};

}