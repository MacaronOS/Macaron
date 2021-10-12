#pragma once

#include "../App/Application.hpp"

namespace UI {

class IntentBase {
public:
    virtual void start() = 0;
};

template <typename ActivityClass>
class Intent : public IntentBase {
public:
    Intent()
    {
    }

    void start() override
    {
        new ActivityClass();
    }
};

}