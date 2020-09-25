#include "assert.hpp"
#include "monitor.hpp"

void ASSERT_PANIC(char* message)
{
    term_print("ASSERT PANIC:\n");
    term_print(message);
    term_print("\n");
    while (1) { }
}

void STOP()
{
    while (1) { }
}