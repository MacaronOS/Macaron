
#include <common.hpp>
#include <extras.hpp>

extern "C" void __cxa_pure_virtual()
{
    while (1)
        ;
}

extern "C" {
static constexpr size_t exit_entries_count = 64;

struct AtExitEntry {
    AtExitFunction method { nullptr };
    void* parameter { nullptr };
    void* dso_handle { nullptr };
};

static struct AtExitEntry atexit_entries[exit_entries_count];
static int atexit_entry_count = 0;

int __cxa_atexit(AtExitFunction exit_function, void* parameter, void* dso_handle)
{
    if (atexit_entry_count >= exit_entries_count) {
        return -1;
    }

    atexit_entries[atexit_entry_count++] = { exit_function, parameter, dso_handle };
    return 0;
}

void __cxa_finalize(void* dso_handle)
{
    for (int at = atexit_entry_count - 1; at >= 0; at--) {
        if (dso_handle && atexit_entries[at].dso_handle != dso_handle) {
            continue;
        }
        atexit_entries[at].method(atexit_entries[at].parameter);
    }
}

static void __atexit_to_cxa_atexit(void* handler)
{
    reinterpret_cast<void (*)()>(handler)();
}

int atexit(void (*handler)())
{
    return __cxa_atexit(__atexit_to_cxa_atexit, (void*)handler, nullptr);
}

}