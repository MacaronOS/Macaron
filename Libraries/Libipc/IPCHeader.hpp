#pragma once

namespace IPC {

struct [[gnu::packed]] IPCHeader {
    int pid_from;
    int pid_to;
    int size;
};

}