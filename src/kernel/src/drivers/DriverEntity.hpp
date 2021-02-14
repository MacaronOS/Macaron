#pragma once

namespace kernel::drivers {

enum class DriverEntity {
    Ata0,
    Ata1,
    Ata2,
    Ata3,

    PIT,

    Uart,

    Keyboard,

    END,
};

}