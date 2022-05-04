#include "vmm.hpp"
#include "Layout.hpp"
#include "Utils.hpp"
#include "pagingstructs.hpp"
#include "pmm.hpp"

#include <Libkernel/Assert.hpp>
#include <Libkernel/Graphics/VgaTUI.hpp>
#include <Libkernel/KError.hpp>
#include <Libkernel/Logger.hpp>
#include <Tasking/Process.hpp>
#include <Tasking/Scheduler.hpp>

#include <Macaronlib/Common.hpp>
#include <Macaronlib/Memory.hpp>

namespace Kernel::Memory {

using namespace Logger;
using namespace Tasking;

VMM::VMM()
    : InterruptHandler(14)
    , m_buffer_1(Layout::GetLocationVirt(LayoutElement::PagingBuffer1))
    , m_buffer_2(Layout::GetLocationVirt(LayoutElement::PagingBuffer2))
{
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
    set_cr3(translation_table_physical_address);
}

void VMM::allocate_pages_from(size_t page, size_t pages, uint32_t flags)
{
    auto& page_directory = m_tranlation_allocator.get_translation_entity<PageDir>(current_translation_table());

    size_t page_directory_index = page / 1024;
    size_t page_table_index = page % 1024;

    assure_page_table(page_directory.entries[page_directory_index], flags);

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
            assure_page_table(page_directory.entries[page_directory_index], flags);
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

        assure_page_table(page_directory_to.entries[page_directory_index], page_directory_from.entries[page_directory_index]._bits & 0x7);

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

    assure_page_table(page_directory.entries[page_directory_index], flags);

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
            assure_page_table(page_directory.entries[page_directory_index], flags);
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

void VMM::allocate_memory_from(uintptr_t address, size_t bytes, uint32_t flags)
{
    allocate_pages_from(address_to_page(address), bytes_to_pages(bytes), flags);
}

void VMM::copy_memory_cow(uintptr_t memory_descriptor_from, uintptr_t address, size_t bytes)
{
    copy_pages_cow(memory_descriptor_from, address_to_page(address), bytes_to_pages(bytes));
}

void VMM::map_memory(uintptr_t virtual_address, uintptr_t physical_address, size_t bytes, uint32_t flags)
{
    map_pages(address_to_page(virtual_address), address_to_page(physical_address), bytes_to_pages(bytes), flags);
}

void VMM::unmap_memory(uintptr_t address, size_t bytes)
{
    unmap_pages(address_to_page(address), bytes_to_pages(bytes));
}

void VMM::handle_interrupt(Trapframe* tf)
{
    constexpr auto pagefault_write_flag = 1 << 1;
    auto address = get_cr2();

    auto try_resolve_page_fault = [&](MemoryDescription& memory_description) -> bool {
        // If no such area is found, this is an incorrect memory reference.
        auto vm_area = memory_description.find_memory_area_for(address);
        if (!vm_area) {
            return false;
        }

        // Check flags
        if (tf->err_code & pagefault_write_flag && !(vm_area->flags() & VM_WRITE)) {
            return false;
        }

        // Check if this page was marked as copy-on-write.
        // Create an individual copy for that page if so.
        if (tf->err_code & pagefault_write_flag) {
            auto pdir_virt = PageBinder<PageDir*>(current_translation_table(), m_buffer_1);
            auto pdentry = pdir_virt.get()->entries[address / CPU::page_size() / 1024];

            if (pdentry._bits) {
                auto ptable_virt = PageBinder<PageTable*>(pdentry.pt_base * CPU::page_size(), m_buffer_2);

                if (ptable_virt.get()->entries[address / CPU::page_size() % 1024]._bits) {
                    if (ptable_virt.get()->entries[address / CPU::page_size() % 1024].rw == false) {
                        auto frame_address = ptable_virt.get()->entries[address / CPU::page_size() % 1024].frame_adress * CPU::page_size();
                        ptable_virt.get()->entries[address / CPU::page_size() % 1024].frame_adress = clone_frame(frame_address) / CPU::page_size();
                        ptable_virt.get()->entries[address / CPU::page_size() % 1024].rw = true;
                        return true;
                    }
                }
            }
        }

        // This is a valid memory reference in the context of VMM.
        // But it still needs to be resolved by a corresponding VMArea object.
        return vm_area->fault(address) == VMArea::PageFaultStatus::Handled;
    };

    if (Scheduler::the().running() && try_resolve_page_fault(Scheduler::the().cur_process()->memory_description())) {
        return;
    }
    if (try_resolve_page_fault(kernel_memory_description)) {
        return;
    }

    Log() << "\n Can not resolve a page fault!\nInformation:\n";

    Log() << "Virtual address: ";
    Log() << address;

    static PageFaultFlag flags[] = {
        PageFaultFlag::Present,
        PageFaultFlag::Write,
        PageFaultFlag::User,
        PageFaultFlag::ReservedWrite,
        PageFaultFlag::InstructionFetch,
    };

    static char* descr[] = {
        "Present",
        "Write",
        "User",
        "ReservedWrite",
        "InstructionFetch",
    };

    Log() << "\nFlags: ";

    for (auto flag : flags) {
        if (tf->err_code & (1 << static_cast<uint32_t>(flag))) {
            Log() << descr[static_cast<uint32_t>(flag)] << " ";
        }
    }

    if (Tasking::Scheduler::the().running()) {
        Log() << "\nPID: " << Tasking::Scheduler::the().cur_process()->id() << "\n";
        auto thread = Tasking::Scheduler::the().cur_thread();
        Log() << "Registers:\n";
        Log() << "eip: " << thread->trapframe()->eip << "\n";
        Log() << "esp: " << thread->trapframe()->useresp << "\n";

        Log() << "stack: " << thread->user_stack() << " " << thread->user_stack_top() << "\n";
    }

    STOP();
}

void VMM::inspect_page_diriectory(uint32_t page_directory_phys)
{
    Log() << "inspecting\n";
    auto page_dir_virt = PageBinder<PageDir*>(page_directory_phys, m_buffer_2);

    for (size_t i = 0; i < 1024; i++) {
        if (i == 768 || i == 769) {
            continue; // skip kernel page tables
        }
        if (page_dir_virt.get()->entries[i]._bits) {
            inspect_page_table(page_dir_virt.get()->entries[i].pt_base * 4096, i);
        }
    }
    Log() << "inspecting done\n";
}

void VMM::inspect_page_table(uint32_t page_table_phys, size_t page_table_index)
{
    auto page_table_virt = PageBinder<PageTable*>(page_table_phys, m_buffer_1);

    for (size_t i = 0; i < 1024; i++) {
        if (page_table_virt.get()->entries[i]._bits) {
            Log() << "Page: " << (page_table_index * 1024 + i) * 4096 << " , Frame: " << page_table_virt.get()->entries[i].frame_adress * 4096 << "\n";
        }
    }
}

}