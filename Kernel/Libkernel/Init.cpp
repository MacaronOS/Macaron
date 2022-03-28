#include "Init.hpp"

namespace Libkernel {

typedef void (*constructor)();
extern "C" constructor start_ctors;
extern "C" constructor end_ctors;

extern "C" void CallConstructors()
{
    for (constructor* i = &start_ctors; i != &end_ctors; i++) {
        (*i)();
    }
}

}