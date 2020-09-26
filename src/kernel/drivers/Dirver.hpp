#pragma once

namespace kernel::drivers {

class Driver {
public:
    Driver() = default;
    ~Driver() = default;

    virtual bool install() { return false; }
    virtual bool uninstall() { return false; }
};

}