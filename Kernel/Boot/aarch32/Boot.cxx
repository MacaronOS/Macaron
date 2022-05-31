
#include <Macaronlib/Common.hpp>
#include <Memory/Layout/Layout.hpp>
#include <Memory/VMM/aarch32/TranslationTables.hpp>
#include <Multiboot.hpp>

extern "C" {

using namespace Kernel;
using namespace Memory;

extern "C" uintptr_t _boot_start;
extern "C" uintptr_t _boot_end;

volatile FirstLevelTranslationTable __attribute__((__aligned__(4 * 4096))) __attribute__((section(".boot_init_bss"))) boot_translation_table {};
volatile SecondLevelTranslationTable __attribute__((section(".boot_init_bss"))) translation_allocator_table_1 {};
volatile SecondLevelTranslationTable __attribute__((section(".boot_init_bss"))) translation_allocator_table_2 {};
volatile SecondLevelTranslationTable __attribute__((section(".boot_init_bss"))) translation_allocator_table_3 {};
volatile SecondLevelTranslationTable __attribute__((section(".boot_init_bss"))) translation_allocator_table_4 {};

void __attribute__((section(".boot_init_text"))) init_boot_translation_table()
{
    // Identity map the first MB. Used to access boot code.
    boot_translation_table.descriptors[(uintptr_t)&_boot_start >> 20] = Section(
        (uintptr_t)&_boot_start >> 20, DescrAP::KernelWrite, DescrExec::Allow)
                                                                            .__bits;

    // Map kernel memory to the higher half using sectoins.
    for (size_t i = 0; i < 4; i++) {
        boot_translation_table.descriptors[(HIGHER_HALF_OFFSET >> 20) + i] = Section(
            ((uintptr_t)&_boot_start >> 20) + i, DescrAP::KernelWrite, DescrExec::Allow)
                                                                                 .__bits;
    }

    // Map kernel translation allocator memory to the higher half using second level tables.
    auto translation_table_descriptor = (HIGHER_HALF_OFFSET + 4 * MB) >> 20;
    auto physical_page = ((uintptr_t)&_boot_start + 4 * MB) / 4096;

    auto map_translation_allocator_table = [&](volatile SecondLevelTranslationTable& table) {
        for (size_t j = 0; j < sizeof(SecondLevelTranslationTable) / sizeof(Descriptor); j++) {
            table.descriptors[j] = SmallPage(physical_page, DescrAP::KernelWrite, DescrExec::Allow).__bits;
            physical_page++;
        }

        boot_translation_table.descriptors[translation_table_descriptor] = PageTable(
            (uintptr_t)&table >> 10, DescrExec::Allow)
                                                                               .__bits;
        translation_table_descriptor++;
    };

    map_translation_allocator_table(translation_allocator_table_1);
    map_translation_allocator_table(translation_allocator_table_2);
    map_translation_allocator_table(translation_allocator_table_3);
    map_translation_allocator_table(translation_allocator_table_4);

    // Map kernel heap memory to the higher half using sectoins.
    for (size_t i = 8; i < 12; i++) {
        boot_translation_table.descriptors[(HIGHER_HALF_OFFSET >> 20) + i] = Section(
            ((uintptr_t)&_boot_start >> 20) + i, DescrAP::KernelWrite, DescrExec::Allow)
                                                                                 .__bits;
    }
}

volatile multiboot_info_t multiboot_structure;
volatile multiboot_memory_map_t multiboot_memory_map[2];

void emulate_multiboot()
{
    multiboot_structure.mem_upper = 0x10000000 / KB; // In KBs.
    // Subtract HIGHER_HALF_OFFSET from the memory map location to make PMM code generic.
    multiboot_structure.mmap_addr = (uintptr_t)&multiboot_memory_map - HIGHER_HALF_OFFSET;
    multiboot_structure.mmap_length = sizeof(multiboot_memory_map);

    multiboot_memory_map[0].addr = 0x80000000;
    multiboot_memory_map[0].len = 0x10000000;
    multiboot_memory_map[0].type = MULTIBOOT_MEMORY_AVAILABLE;
    multiboot_memory_map[0].size = sizeof(multiboot_memory_map_t);

    multiboot_memory_map[1].addr = (uintptr_t)&_boot_start;
    multiboot_memory_map[1].len = (uintptr_t)&_boot_end - (uintptr_t)&_boot_start;
    multiboot_memory_map[1].type = MULTIBOOT_MEMORY_RESERVED;
    multiboot_memory_map[1].size = sizeof(multiboot_memory_map_t);
}
}