#include "../VMM.hpp"
#include "TranslationTables.hpp"

#include <Hardware/x86/CPU.hpp>
#include <Libkernel/Assert.hpp>
#include <Libkernel/KError.hpp>
#include <Libkernel/Logger.hpp>
#include <Memory/pmm.hpp>
#include <Tasking/Scheduler/Scheduler.hpp>

#include <Macaronlib/Common.hpp>
#include <Macaronlib/Memory.hpp>

namespace Kernel::Memory {

extern "C" volatile PageDir boot_page_directory;

using namespace Logger;
using namespace Tasking;

static inline void assure_page_table(PDEntry& pde, uint32_t flags, TranslationAllocator& alloc)
{
    if (!pde._bits) {
        auto& page_table = alloc.allocate_tranlation_entity<PageTable>();
        pde.pt_base = alloc.virtual_to_physical((uintptr_t)&page_table) / CPU::page_size();
        pde._bits |= (flags & 0x7);
    }
}

uintptr_t VMM::current_translation_table() const
{
    uintptr_t cr3;
    asm("mov %%cr3, %%eax"
        : "=a"(cr3));
    return cr3;
}

uintptr_t VMM::create_translation_table()
{
    auto& kpage_directory = *reinterpret_cast<PageDir*>(Layout::PhysToVirt((uintptr_t)&boot_page_directory));
    auto& page_directory = m_tranlation_allocator.allocate_tranlation_entity<PageDir>();

    page_directory.entries[768] = kpage_directory.entries[768];
    page_directory.entries[769] = kpage_directory.entries[769];
    page_directory.entries[770] = kpage_directory.entries[770];

    return m_tranlation_allocator.virtual_to_physical((uintptr_t)&page_directory);
}

void VMM::set_translation_table(uintptr_t translation_table_physical_address)
{
    if (current_translation_table() == translation_table_physical_address) {
        return;
    }
    asm volatile("mov %%eax, %%cr3" ::"a"(translation_table_physical_address)
                 : "memory");
}

void VMM::allocate_pages_from(size_t page, size_t pages, uint32_t flags)
{
    auto& page_directory = m_tranlation_allocator.get_translation_entity<PageDir>(current_translation_table());

    size_t page_directory_index = page / 1024;
    size_t page_table_index = page % 1024;

    assure_page_table(page_directory.entries[page_directory_index], flags, m_tranlation_allocator);

    size_t pages_left = pages;
    for (; pages_left > 0; pages_left--) {
        auto page_table_physical_address = page_directory.entries[page_directory_index].pt_base * CPU::page_size();
        auto& page_table = m_tranlation_allocator.get_translation_entity<PageTable>(page_table_physical_address);

        if (!page_table.entries[page_table_index]._bits) {
            page_table.entries[page_table_index].frame_adress = PMM::the().allocate_frame();
            page_table.entries[page_table_index]._bits |= (flags & 0x7);
        }

        page_table_index++;
        if (page_table_index >= 1024) {
            page_table_index = 0;
            page_directory_index++;
            assure_page_table(page_directory.entries[page_directory_index], flags, m_tranlation_allocator);
        }
    }

    CPU::flush_tlb(page * CPU::page_size(), pages);
}

void VMM::copy_pages_cow(uintptr_t translation_table_from, size_t page, size_t pages)
{
    auto& page_directory_to = m_tranlation_allocator.get_translation_entity<PageDir>(current_translation_table());
    auto& page_directory_from = m_tranlation_allocator.get_translation_entity<PageDir>(translation_table_from);

    size_t page_directory_index = page / 1024;
    size_t page_table_index = page % 1024;

    size_t pages_left = pages;
    for (; pages_left > 0;) {
        // Skip an entire page table, if there's no page table in "from" page directory.
        if (!page_directory_from.entries[page_directory_index]._bits) {
            pages_left -= min(pages_left, 1024);
            page_table_index = 0;
            page_directory_index++;
            continue;
        }

        assure_page_table(
            page_directory_to.entries[page_directory_index],
            page_directory_from.entries[page_directory_index]._bits & 0x7,
            m_tranlation_allocator);

        auto page_table_to_physical_address = page_directory_to.entries[page_directory_index].pt_base * CPU::page_size();
        auto page_table_from_physical_address = page_directory_from.entries[page_directory_index].pt_base * CPU::page_size();

        auto& page_table_to = m_tranlation_allocator.get_translation_entity<PageTable>(page_table_to_physical_address);
        auto& page_table_from = m_tranlation_allocator.get_translation_entity<PageTable>(page_table_from_physical_address);

        if (page_table_from.entries[page_table_index]._bits) {
            // "from" page is copy-on-write now.
            page_table_from.entries[page_table_index].rw = false;
            page_table_to.entries[page_table_index] = page_table_from.entries[page_table_index];
        }

        pages_left--;
        page_table_index++;
        if (page_table_index >= 1024) {
            page_table_index = 0;
            page_directory_index++;
        }
    }

    CPU::flush_tlb(page * CPU::page_size(), pages);
}

void VMM::map_pages(size_t page, size_t frame, size_t pages, uint32_t flags)
{
    auto& page_directory = m_tranlation_allocator.get_translation_entity<PageDir>(current_translation_table());

    size_t page_directory_index = page / 1024;
    size_t page_table_index = page % 1024;

    assure_page_table(page_directory.entries[page_directory_index], flags, m_tranlation_allocator);

    size_t pages_left = pages;
    for (; pages_left > 0; pages_left--, frame++) {
        auto page_table_physical_address = page_directory.entries[page_directory_index].pt_base * CPU::page_size();
        auto& page_table = m_tranlation_allocator.get_translation_entity<PageTable>(page_table_physical_address);

        if (!page_table.entries[page_table_index]._bits) {
            page_table.entries[page_table_index].frame_adress = frame;
            page_table.entries[page_table_index]._bits |= (flags & 0x7);
        }

        page_table_index++;
        if (page_table_index >= 1024) {
            page_table_index = 0;
            page_directory_index++;
            assure_page_table(page_directory.entries[page_directory_index], flags, m_tranlation_allocator);
        }
    }

    CPU::flush_tlb(page * CPU::page_size(), pages);
}

void VMM::unmap_pages(size_t page, size_t pages)
{
    auto& page_directory = m_tranlation_allocator.get_translation_entity<PageDir>(current_translation_table());

    size_t page_directory_index = page / 1024;
    size_t page_table_index = page % 1024;

    size_t pages_left = pages;
    for (; pages_left > 0;) {
        if (!page_directory.entries[page_directory_index]._bits) {
            pages_left -= min(pages_left, 1024);
            page_table_index = 0;
            page_directory_index++;
            continue;
        }

        auto page_table_physical_address = page_directory.entries[page_directory_index].pt_base * CPU::page_size();
        auto& page_table = m_tranlation_allocator.get_translation_entity<PageTable>(page_table_physical_address);

        if (page_table.entries[page_table_index]._bits) {
            page_table.entries[page_table_index]._bits = 0;
        }

        pages_left--;
        page_table_index++;
        if (page_table_index >= 1024) {
            page_table_index = 0;
            page_directory_index++;
        }
    }

    CPU::flush_tlb(page * CPU::page_size(), pages);
}

class VMMInterruptHandler : public InterruptHandler {
public:
    VMMInterruptHandler()
        : InterruptHandler(14)
    {
    }

    //^InterruptHandler
    void handle_interrupt(Trapframe* tf)
    {
        VMM::the().on_fault(read_page_fault_linear_address(), tf->err_code);
    }
};

static VMMInterruptHandler _;

void VMM::on_fault(uintptr_t address, uint32_t flags)
{
    constexpr auto pagefault_write_flag = 1 << 1;

    auto try_resolve_page_fault = [&](MemoryDescription& memory_description) -> bool {
        // If no such area is found, this is an incorrect memory reference.
        auto vm_area = memory_description.find_memory_area_for(address);
        if (!vm_area) {
            return false;
        }

        // Check flags
        if (flags & pagefault_write_flag && !(vm_area->flags() & VM_WRITE)) {
            return false;
        }

        // Check if this page was marked as copy-on-write.
        // Create an individual copy for that page if so.
        if (flags & pagefault_write_flag) {
            auto& page_directory = m_tranlation_allocator.get_translation_entity<PageDir>(current_translation_table());
            auto& page_directory_entry = page_directory.entries[address / CPU::page_size() / 1024];

            if (page_directory_entry._bits) {
                auto page_table_physical_address = page_directory_entry.pt_base * CPU::page_size();
                auto& page_table = m_tranlation_allocator.get_translation_entity<PageTable>(page_table_physical_address);
                auto& page_table_entry = page_table.entries[address / CPU::page_size() % 1024];

                if (page_table_entry._bits) {
                    if (page_table_entry.rw == false) {
                        auto frame_address = page_table_entry.frame_adress * CPU::page_size();
                        page_table_entry.frame_adress = clone_frame_of_address(frame_address) / CPU::page_size();
                        page_table_entry.rw = true;
                        CPU::flush_tlb(address, 1);
                        return true;
                    }
                }
            }
        }

        // This is a valid memory reference in the context of VMM.
        // But it still needs to be resolved by a corresponding VMArea object.
        return vm_area->fault(address) == VMArea::PageFaultStatus::Handled;
    };

    if (Scheduler::the().running() && try_resolve_page_fault(Scheduler::the().current_process().memory_description())) {
        return;
    }
    if (try_resolve_page_fault(kernel_memory_description)) {
        return;
    }

    Log() << "\n Can not resolve a page fault!\nInformation:\n";

    Log() << "Virtual address: ";
    Log() << address;

    static PageFaultFlag page_fault_flags[] = {
        PageFaultFlag::Present,
        PageFaultFlag::Write,
        PageFaultFlag::User,
        PageFaultFlag::ReservedWrite,
        PageFaultFlag::InstructionFetch,
    };

    static const char* descr[] = {
        "Present",
        "Write",
        "User",
        "ReservedWrite",
        "InstructionFetch",
    };

    Log() << "\nFlags: ";

    for (auto flag : page_fault_flags) {
        if (flags & (1 << static_cast<uint32_t>(flag))) {
            Log() << descr[static_cast<uint32_t>(flag)] << " ";
        }
    }

    if (Tasking::Scheduler::the().running()) {
        Log() << "\nPID: " << Tasking::Scheduler::the().current_process().id() << "\n";
        auto& thread = Tasking::Scheduler::the().current_thread();
        Log() << "Registers:\n";
        Log() << "eip: " << thread.trapframe()->eip << "\n";
        Log() << "esp: " << thread.trapframe()->useresp << "\n";
    }

    STOP();
}

}