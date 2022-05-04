#include "Init.hpp"
#include <Hardware/CPU.hpp>
#include <Macaronlib/Common.hpp>
#include <Memory/Layout.hpp>
#include <Memory/pagingstructs.hpp>
#include <Memory/vmm.hpp>

namespace Libkernel {

using namespace Kernel;
using namespace Kernel::Memory;

typedef void (*constructor)();
extern "C" constructor start_ctors;
extern "C" constructor end_ctors;

extern "C" void CallConstructors()
{
    for (constructor* i = &start_ctors; i != &end_ctors; i++) {
        (*i)();
    }
}

extern "C" {

volatile PageDir __attribute__((section(".boot_init_bss"))) boot_page_directory {};
volatile PageTable __attribute__((section(".boot_init_bss"))) kernel_page_table {};
volatile PageTable __attribute__((section(".boot_init_bss"))) kernel_translation_allocator_page_table {};
volatile PageTable __attribute__((section(".boot_init_bss"))) kernel_heap_page_table {};

void __attribute__((section(".boot_init_text"))) init_boot_translation_table()
{
    auto page_directory_entry = HIGHER_HALF_OFFSET / CPU::page_size() / 1024;
    auto frame = 0;

    auto map_table = [&page_directory_entry, &frame](volatile PageTable& page_table) {
        for (size_t i = 0; i < 1024; i++) {
            // Map frames to the table.
            page_table.entries[i].present = 1;
            page_table.entries[i].rw = 1;
            page_table.entries[i].user_mode = 1;
            page_table.entries[i].frame_adress = frame++;
        }

        // Map a table itself.
        boot_page_directory.entries[page_directory_entry].present = 1;
        boot_page_directory.entries[page_directory_entry].rw = 1;
        boot_page_directory.entries[page_directory_entry].user_mode = 1;
        boot_page_directory.entries[page_directory_entry].pt_base = (uintptr_t)&page_table / CPU::page_size();

        page_directory_entry++;
    };

    map_table(kernel_page_table);
    map_table(kernel_translation_allocator_page_table);
    map_table(kernel_heap_page_table);

    // Identity map the first table. Used during boot.
    boot_page_directory.entries[0].present = 1;
    boot_page_directory.entries[0].rw = 1;
    boot_page_directory.entries[0].user_mode = 1;
    boot_page_directory.entries[0].pt_base = (uint32_t)&kernel_page_table / CPU::page_size();
}
}

}