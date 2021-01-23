// An entry point for tests

#pragma once
#include "Tester.hpp"
#include "fs/vfs.test.hpp"

void test_main() 
{
    auto tester = kernel::tests::Tester();

    tester.add_test(new kernel::tests::TestVFS());

    tester.run_all();
}