#include "Init.hpp"
#include <Macaronlib/Common.hpp>
#include <Memory/Layout.hpp>
#include <Memory/pagingstructs.hpp>
#include <Memory/vmm.hpp>

namespace Libkernel {

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
volatile PageTable __attribute__((section(".boot_init_bss"))) boot_page_table1 {};
volatile PageTable __attribute__((section(".boot_init_bss"))) boot_page_table2 {};

void __attribute__((section(".boot_init_text"))) init_boot_translation_table()
{
    for (size_t i = 0; i < 1024; i++) {
        boot_page_table1.entries[i].present = 1;
        boot_page_table1.entries[i].rw = 1;
        boot_page_table1.entries[i].user_mode = 1;
        boot_page_table1.entries[i].frame_adress = i;
    }

    for (size_t i = 0; i < 1024; i++) {
        boot_page_table2.entries[i].present = 1;
        boot_page_table2.entries[i].rw = 1;
        boot_page_table2.entries[i].user_mode = 1;
        boot_page_table2.entries[i].frame_adress = 1024 + i;
    }

    // Identity mapping of the first 4MB.
    boot_page_directory.entries[0].present = 1;
    boot_page_directory.entries[0].rw = 1;
    boot_page_directory.entries[0].user_mode = 1;
    boot_page_directory.entries[0].pt_base = (uint32_t)&boot_page_table1 / PAGE_SIZE;

    // Map 8MB of the kernel memory to the higher half.
    auto pd_entry = HIGHER_HALF_OFFSET / PAGE_SIZE / 1024;

    boot_page_directory.entries[pd_entry].present = 1;
    boot_page_directory.entries[pd_entry].rw = 1;
    boot_page_directory.entries[pd_entry].user_mode = 1;
    boot_page_directory.entries[pd_entry].pt_base = (uint32_t)&boot_page_table1 / PAGE_SIZE;

    boot_page_directory.entries[pd_entry + 1].present = 1;
    boot_page_directory.entries[pd_entry + 1].rw = 1;
    boot_page_directory.entries[pd_entry + 1].user_mode = 1;
    boot_page_directory.entries[pd_entry + 1].pt_base = (uint32_t)&boot_page_table2 / PAGE_SIZE;
}

}

}