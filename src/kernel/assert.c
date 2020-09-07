#include "assert.h"
#include "monitor.h"

void ASSERT_PANIC(char* message) {
    term_print("ASSERT PANIC:\n");
    term_print(message);
    term_print("\n");
    while (1);
}