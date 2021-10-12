// Interface to global information about an application environment.

#pragma once

#include "Intent.hpp"

namespace UI {

class Context {
public:
    virtual void start_activity(IntentBase& intent)
    {
        intent.start();
    }
};

}