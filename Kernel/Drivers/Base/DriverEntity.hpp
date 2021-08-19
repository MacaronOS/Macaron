#pragma once

namespace Kernel::Drivers {

enum class DriverEntity {
    Ata0,
    Ata1,
    Ata2,
    Ata3,

    Uart,

    Mouse,
    Keyboard,

    PIT,

    PCI,

    BGA,

    END,
};

}