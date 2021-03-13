#include "Tester.hpp"
#include "../monitor.hpp"

namespace kernel::tests {

void Tester::add_test(Test* tests)
{
    m_tests.push_back(tests);
}

bool Tester::run_all()
{
    term_print("Wisteria Tester\n");
    bool ok = true;

    for (size_t i = 0; i < m_tests.size(); i++) {
        bool test_res = m_tests[i]->test();

        term_print(m_tests[i]->name());
        if (test_res) {
            term_print(": OK\n");
        } else {
            term_print(": FAIL\n");
        }
        ok &= test_res;
    }

    if (ok) {
        term_print("RESULT: OK\n");
    } else {
        term_print("RESULT: FAIL\n");
    }

    return ok;
}

};