#include "../VMM.hpp"
#include "TranslationTables.hpp"
#include <Hardware/aarch32/CPU.hpp>
#include <Libkernel/Logger.hpp>

namespace Kernel::Memory {

extern "C" volatile FirstLevelTranslationTable boot_translation_table;

template <typename TranslationEntity>
void assure_tranlation_entity(void* to, TranslationEntity* from, TranslationAllocator& alloc)
{
    auto to_entity = static_cast<TranslationEntity*>(to);

    if (to_entity->__bits == from->__bits) {
        return;
    }

    *to_entity = *from;

    if constexpr (IsSame<TranslationEntity, PageTable>) {
        auto& second_level_translation_table = alloc.allocate_tranlation_entity<SecondLevelTranslationTable>();
        auto page_table_base_address = (uintptr_t)&second_level_translation_table >> 10;
        to_entity->base_address = page_table_base_address;
        return;
    }

    if constexpr (IsSame<TranslationEntity, Section>) {
        auto section_base_address = PMM::the().allocate_frames(1 * MB / CPU::page_size()) >> 12;
        to_entity->base_address = section_base_address;
        return;
    }

    if constexpr (IsSame<TranslationEntity, Supersection>) {
        auto supersection_base_address = PMM::the().allocate_frames(12 * MB / CPU::page_size()) >> 12;
        to_entity->base_address = supersection_base_address;
        return;
    }

    if constexpr (IsSame<TranslationEntity, SmallPage>) {
        auto small_page_base_address = PMM::the().allocate_frame() >> 12;
        to_entity->base_address = small_page_base_address;
        return;
    }

    static_assert("[VMM][Aarch32] Unsupported translation entity");
}

uintptr_t VMM::current_translation_table() const
{
    uint32_t ttbr0;
    asm volatile("mrc p15, 0, %0, c2, c0, 0"
                 : "=r"(ttbr0)
                 :);
    return ttbr0;
}

uintptr_t VMM::create_translation_table()
{
    auto& ktranslation_table = *reinterpret_cast<FirstLevelTranslationTable*>(
        Layout::PhysToVirt((uintptr_t)&boot_translation_table));

    auto& translation_table = m_tranlation_allocator.allocate_tranlation_entity<FirstLevelTranslationTable>();

    // Copy 12 entries from boot first level translation table to the new one.
    // See Boot/aarch32/Boot.cxx - init_boot_translation_table();
    for (size_t i = 0; i < 12; i++) {
        translation_table.descriptors[(HIGHER_HALF_OFFSET >> 20) + i] = ktranslation_table.descriptors[(HIGHER_HALF_OFFSET >> 20) + i];
    }

    translation_table = ktranslation_table;

    return m_tranlation_allocator.virtual_to_physical((uintptr_t)&translation_table);
}

void VMM::set_translation_table(uintptr_t translation_table_physical_address)
{
    if (current_translation_table() == translation_table_physical_address) {
        return;
    }
    asm volatile("mcr p15, 0, %0, c2, c0, 0"
                 :
                 : "r"(translation_table_physical_address)
                 : "memory");
    asm volatile("dmb");
}

void VMM::allocate_pages_from(size_t page, size_t pages, uint32_t flags)
{
    auto& first_level_translation_table = m_tranlation_allocator.get_translation_entity<FirstLevelTranslationTable>(current_translation_table());

    size_t current_page = page;

    for (; current_page - page < pages;) {
        size_t first_level_translation_table_index = page / 256;
        auto first_level_description = &first_level_translation_table.descriptors[first_level_translation_table_index];

        if (*first_level_description == 0) {
            auto& page_table = m_tranlation_allocator.allocate_tranlation_entity<SecondLevelTranslationTable>();
            auto page_table_base_address = m_tranlation_allocator.virtual_to_physical((uintptr_t)&page_table) >> 10;
            *first_level_description = PageTable(page_table_base_address, DescrExec::Allow).__bits;
        }

        if (auto page_table_description = PageTable::from(first_level_description); page_table_description->match()) {
            auto page_table_physical_address = page_table_description->base_address * 1024;
            auto& second_level_translation_table = m_tranlation_allocator.get_translation_entity<SecondLevelTranslationTable>(page_table_physical_address);

            size_t second_level_translation_table_index = page % 256;
            auto second_level_description = &second_level_translation_table.descriptors[second_level_translation_table_index];

            if (*second_level_description == 0) {
                *second_level_description = SmallPage(PMM::the().allocate_frame(), DescrAP::UserWrite, DescrExec::Allow).__bits;
            }

            if (auto small_page_description = SmallPage::from(second_level_description); small_page_description->match()) {
                current_page++;
                continue;
            }

            ASSERT_PANIC("[VMM][aarch32] Met unexpected second level descriptor");
        }

        if (auto section_description = Section::from(first_level_description); section_description->match()) {
            current_page += 1 * MB / CPU::page_size();
            continue;
        }

        if (auto supersection_description = Supersection::from(first_level_description); supersection_description->match()) {
            current_page += 12 * MB / CPU::page_size();
            continue;
        }
    }

    CPU::flush_tlb(page * CPU::page_size(), pages);
}

void VMM::copy_pages_cow(uintptr_t translation_table_from, size_t page, size_t pages)
{
    auto& first_level_translation_table_to = m_tranlation_allocator.get_translation_entity<FirstLevelTranslationTable>(current_translation_table());
    auto& first_level_translation_table_from = m_tranlation_allocator.get_translation_entity<FirstLevelTranslationTable>(translation_table_from);

    size_t current_page = page;

    for (; current_page - page < pages;) {
        size_t first_level_translation_table_index = page / 256;
        auto first_level_description_from = &first_level_translation_table_from.descriptors[first_level_translation_table_index];

        // Move to the next first_level_translation_table_index.
        if (*first_level_description_from == 0) {
            current_page += 256;
            continue;
        }

        auto first_level_description_to = &first_level_translation_table_to.descriptors[first_level_translation_table_index];

        if (auto page_table_description_from = PageTable::from(first_level_description_from); page_table_description_from->match()) {
            assure_tranlation_entity(first_level_description_to, page_table_description_from, m_tranlation_allocator);

            auto page_table_physical_address_from = page_table_description_from->base_address << 10;
            auto& second_level_translation_table_from = m_tranlation_allocator.get_translation_entity<SecondLevelTranslationTable>(page_table_physical_address_from);

            size_t second_level_translation_table_index = page % 256;
            auto second_level_description_from = &second_level_translation_table_from.descriptors[second_level_translation_table_index];

            // Move ot the next second_level_translation_table_index.
            if (*second_level_description_from == 0) {
                current_page += 1;
                continue;
            }

            auto page_table_description_to = PageTable::from(first_level_description_to);
            auto page_table_physical_address_to = page_table_description_to->base_address << 10;
            auto& second_level_translation_table_to = m_tranlation_allocator.get_translation_entity<SecondLevelTranslationTable>(page_table_physical_address_to);

            if (auto small_page_description_from = SmallPage::from(second_level_description_from); small_page_description_from->match()) {
                // small page is copy-on-write now.
                small_page_description_from->set_access_permissions(DescrAP::UserRead);
                auto smal_page_description_to = &second_level_translation_table_to.descriptors[second_level_translation_table_index];
                assure_tranlation_entity(smal_page_description_to, small_page_description_from, m_tranlation_allocator);
                current_page += 1;
                continue;
            }

            ASSERT_PANIC("[VMM][aarch32] Met unexpected second level descriptor");
        }

        if (auto section_description_from = Section::from(first_level_description_from); section_description_from->match()) {
            // section is copy-on-write now.
            section_description_from->set_access_permissions(DescrAP::UserRead);
            assure_tranlation_entity(first_level_description_to, section_description_from, m_tranlation_allocator);
            current_page += 1 * MB / CPU::page_size();
            continue;
        }

        if (auto supersection_description_from = Supersection::from(first_level_description_from); supersection_description_from->match()) {
            // supersection is copy-on-write now.
            supersection_description_from->set_access_permissions(DescrAP::UserRead);
            assure_tranlation_entity(first_level_description_to, supersection_description_from, m_tranlation_allocator);
            current_page += 12 * MB / CPU::page_size();
            continue;
        }

        ASSERT_PANIC("[VMM][aarch32] Met unexpected first level descriptor");
    }

    CPU::flush_tlb(page * CPU::page_size(), pages);
}

void VMM::map_pages(size_t page, size_t frame, size_t pages, uint32_t flags)
{
    auto& first_level_translation_table = m_tranlation_allocator.get_translation_entity<FirstLevelTranslationTable>(current_translation_table());

    size_t current_page = page;

    for (; current_page - page < pages;) {
        size_t first_level_translation_table_index = page / 256;
        auto first_level_description = &first_level_translation_table.descriptors[first_level_translation_table_index];

        if (*first_level_description == 0) {
            auto& page_table = m_tranlation_allocator.allocate_tranlation_entity<SecondLevelTranslationTable>();
            auto page_table_base_address = m_tranlation_allocator.virtual_to_physical((uintptr_t)&page_table) >> 10;
            *first_level_description = PageTable(page_table_base_address, DescrExec::Allow).__bits;
        }

        if (auto page_table_description = PageTable::from(first_level_description); page_table_description->match()) {
            auto page_table_physical_address = page_table_description->base_address * 1024;
            auto& second_level_translation_table = m_tranlation_allocator.get_translation_entity<SecondLevelTranslationTable>(page_table_physical_address);

            size_t second_level_translation_table_index = page % 256;
            auto second_level_description = &second_level_translation_table.descriptors[second_level_translation_table_index];

            if (*second_level_description == 0) {
                *second_level_description = SmallPage(frame, DescrAP::UserWrite, DescrExec::Allow).__bits;
                frame++;
                current_page++;
                continue;
            }

            if (auto small_page_description = SmallPage::from(second_level_description); small_page_description->match()) {
                ASSERT_PANIC("[VMM][aarch32] Met unexpected small page descriptor in map");
            }

            ASSERT_PANIC("[VMM][aarch32] Met unexpected second level descriptor");
        }

        if (auto section_description = Section::from(first_level_description); section_description->match()) {
            ASSERT_PANIC("[VMM][aarch32] Met unexpected section descriptor in map");
        }

        if (auto supersection_description = Supersection::from(first_level_description); supersection_description->match()) {
            ASSERT_PANIC("[VMM][aarch32] Met unexpected supersection descriptor in map");
        }

        ASSERT_PANIC("[VMM][aarch32] Met unexpected first level descriptor");
    }

    CPU::flush_tlb(page * CPU::page_size(), pages);
}

void VMM::unmap_pages(size_t page, size_t pages)
{
    auto& first_level_translation_table = m_tranlation_allocator.get_translation_entity<FirstLevelTranslationTable>(current_translation_table());

    size_t current_page = page;

    for (; current_page - page < pages;) {
        size_t first_level_translation_table_index = page / 256;
        auto first_level_description = &first_level_translation_table.descriptors[first_level_translation_table_index];

        // Move to the next first_level_translation_table_index.
        if (*first_level_description == 0) {
            current_page += 256;
            continue;
        }

        if (auto page_table_description = PageTable::from(first_level_description); page_table_description->match()) {
            auto page_table_physical_address = page_table_description->base_address * 1024;
            auto& second_level_translation_table = m_tranlation_allocator.get_translation_entity<SecondLevelTranslationTable>(page_table_physical_address);

            size_t second_level_translation_table_index = page % 256;
            auto second_level_description = &second_level_translation_table.descriptors[second_level_translation_table_index];

            // Move ot the next second_level_translation_table_index.
            if (*second_level_description == 0) {
                current_page += 1;
                continue;
            }

            if (auto small_page_description = SmallPage::from(second_level_description); small_page_description->match()) {
                small_page_description->__bits = 0;
                current_page += 1;
                continue;
            }

            ASSERT_PANIC("[VMM][aarch32] Met unexpected second level descriptor");
        }

        if (auto section_description = Section::from(first_level_description); section_description->match()) {
            section_description->__bits = 0;
            current_page += 1 * MB / CPU::page_size();
            continue;
        }

        if (auto supersection_description = Supersection::from(first_level_description); supersection_description->match()) {
            supersection_description->__bits = 0;
            current_page += 12 * MB / CPU::page_size();
            continue;
        }

        ASSERT_PANIC("[VMM][aarch32] Met unexpected first level descriptor");
    }

    CPU::flush_tlb(page * CPU::page_size(), pages);
}

namespace FaultStatus {
    enum FaultStatus {
        AlignmentFault = 00001,
        InstructionCacheMaintenanceFault = 00100,
        SynchronousExternalAbortFirstLevel = 01100,
        SynchronousExternalAbortSecondLevel = 01110,
        SynchronousParityErrorFirstLevel = 11100,
        SynchronousParityErrorSecondLevel = 11110,
        TranslationFaultFirstLevel = 00101,
        TranslationFaultSecondLevel = 00111,
        AccessFlagFaultFirstLevel = 00011,
        AccessFlagFaultSecondLevel = 00110,
        DomainFaultFirstLevel = 01001,
        DomainFaultSecondLevel = 01011,
        PermissionFaultFirstLevel = 01101,
        PermissionFaultSecondLevel = 01111,
        DebugeEvent = 00010,
        SynchronousExternalAbort = 01000,
        TLBConflictAbort = 10000,
        SynchronousParityError = 11001,
        AsynchronousExternalAbort = 10110,
        AsynchronousParityError = 11000,
    };
}

void VMM::on_fault(uintptr_t address, uint32_t flags_bits)
{
    DFSR flags;
    flags.__bits = flags_bits;
    Log() << "\nCan not resolve a page fault!\nInformation:\n";
    Log() << "Virtual address: " << address << "\n";
    Log() << "Flags: " << flags.__bits << "\n";

    STOP();
}

}