#pragma once

#ifdef __i386__
#include "x86/Trapframe.hpp"
#else
#include "aarch32/Trapframe.hpp"
#endif