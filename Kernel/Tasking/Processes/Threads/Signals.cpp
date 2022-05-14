#include "Signals.hpp"
#include <Libkernel/Assert.hpp>
#include <Memory/VMM/VMM.hpp>
#include <Tasking/MemoryDescription/MemoryDescription.hpp>

namespace Kernel::Tasking {

using namespace Memory;

extern "C" void signal_caller();
extern "C" void signal_caller_end();

static SharedVMArea* s_signal_caller_area;

void Signals::setup_caller()
{
    size_t signal_caller_len = (uint32_t)signal_caller_end - (uint32_t)signal_caller;
    auto signal_area = kernel_memory_description.allocate_memory_area<SharedVMArea>(
        signal_caller_len,
        VM_READ | VM_WRITE | VM_EXEC,
        true);

    if (!signal_area) {
        ASSERT_PANIC("[Signals] Could not allocate signal caller vmarea");
    }

    VMM::the().set_translation_table(kernel_memory_description.memory_descriptor());
    memcpy((void*)signal_area.result()->vm_start(), (void*)signal_caller, signal_caller_len);
    s_signal_caller_area = signal_area.result();
}

uintptr_t Signals::caller_ip()
{
    return s_signal_caller_area->vm_start();
}

}