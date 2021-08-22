#include "vmm.hpp"
#include "Layout.hpp"
#include "pagingstructs.hpp"
#include "pmm.hpp"

#include <Libkernel/Logger.hpp>
#include <Libkernel/Assert.hpp>
#include <Libkernel/KError.hpp>
#include <Libkernel/Graphics/VgaTUI.hpp>
#include <Tasking/TaskManager.hpp>

#include <Wisterialib/Common.hpp>
#include <Wisterialib/Memory.hpp>

namespace Kernel::Memory {

using namespace Logger;
using namespace Tasking;

template <>
VMM* Singleton<VMM>::s_t = nullptr;
template <>
bool Singleton<VMM>::s_initialized = false;

VMM::VMM()
    : InterruptHandler(14)
    , m_buffer_1(Layout::GetLocationVirt(LayoutElement::PagingBuffer1))
    , m_buffer_2(Layout::GetLocationVirt(LayoutElement::PagingBuffer2))
{
}

void VMM::set_page_directory(uint32_t page_directory_phys)
{
    set_cr3(page_directory_phys);
    m_cur_page_dir_phys = page_directory_phys;
}

void VMM::psized_map(uint32_t pdir_phys, uint32_t page, uint32_t frame, uint32_t pages, uint32_t flags)
{
    auto pdir_virt = PageBinder<PageDir*>(pdir_phys, m_buffer_1);

    uint32_t pdir_index = page / 1024;
    uint32_t pt_index = page % 1024;

    create_ptable_if_neccesary(pdir_virt.get()->entries[pdir_index], flags);

    auto ptable_virt = PageBinder<PageTable*>(pdir_virt.get()->entries[pdir_index].pt_base * PAGE_SIZE, m_buffer_2);

    for (; pages > 0; pages--, page++, frame++) {
        ptable_virt.get()->entries[pt_index].frame_adress = frame;
        ptable_virt.get()->entries[pt_index]._bits |= (flags & 0x7);
        pt_index++;
        if (pt_index >= 1024) {
            pt_index = 0;
            pdir_index++;
            create_ptable_if_neccesary(pdir_virt.get()->entries[pdir_index], flags);
            ptable_virt.rebind(pdir_virt.get()->entries[pdir_index].pt_base * PAGE_SIZE);
        }
    }
}

void VMM::psized_unmap(uint32_t pdir_phys, uint32_t page, uint32_t pages)
{
    auto pdir_virt = PageBinder<PageDir*>(pdir_phys, m_buffer_1);

    uint32_t pdir_index = page / 1024;
    uint32_t pt_index = page % 1024;

    auto ptable_virt = PageBinder<PageTable*>(pdir_virt.get()->entries[pdir_index].pt_base * PAGE_SIZE, m_buffer_2);

    for (; pages > 0; pages--, page++) {
        ptable_virt.get()->entries[pt_index]._bits = 0;
        pt_index++;
        if (pt_index >= 1024) {
            pt_index = 0;
            pdir_index++;
            ptable_virt.rebind(pdir_virt.get()->entries[pdir_index].pt_base * PAGE_SIZE);
        }
    }
}

void VMM::psized_copy(uint32_t pdir_phys_to, uint32_t pdir_phys_from, uint32_t page, uint32_t pages)
{
    auto pdir_virt_to = PageBinder<PageDir*>(pdir_phys_to, m_buffer_1);
    auto pdir_virt_from = PageBinder<PageDir*>(pdir_phys_from, m_buffer_2);

    uint32_t pdir_index = page / 1024;
    uint32_t pt_index = page % 1024;

    create_ptable_if_neccesary(pdir_virt_to.get()->entries[pdir_index], pdir_virt_from.get()->entries[pdir_index]._bits & 0x7);

    for (; pages > 0; pages--, page++) {
        {
            auto ptable_virt_to = PageBinder<PageTable*>(pdir_virt_to.get()->entries[pdir_index].pt_base * PAGE_SIZE, m_buffer_1);
            auto ptable_virt_from = PageBinder<PageTable*>(pdir_virt_from.get()->entries[pdir_index].pt_base * PAGE_SIZE, m_buffer_2);
            ptable_virt_to.get()->entries[pt_index] = ptable_virt_from.get()->entries[pt_index];
            ptable_virt_to.get()->entries[pt_index].frame_adress = clone_frame(ptable_virt_from.get()->entries[pt_index].frame_adress * PAGE_SIZE) / PAGE_SIZE;
        }
        pt_index++;
        if (pt_index >= 1024) {
            pt_index = 0;
            pdir_index++;
            create_ptable_if_neccesary(pdir_virt_to.get()->entries[pdir_index], pdir_virt_from.get()->entries[pdir_index]._bits & 0x7);
        }
    }
}

void VMM::psized_free(uint32_t pdir_phys, uint32_t page, uint32_t pages)
{
    auto pdir_virt = PageBinder<PageDir*>(pdir_phys, m_buffer_1);

    uint32_t pdir_index = page / 1024;
    uint32_t pt_index = page % 1024;

    auto ptable_virt = PageBinder<PageTable*>(pdir_virt.get()->entries[pdir_index].pt_base * PAGE_SIZE, m_buffer_2);

    for (; pages > 0; pages--, page++) {
        free_frame(ptable_virt.get()->entries[pt_index].frame_adress);
        ptable_virt.get()->entries[pt_index]._bits = 0;
        pt_index++;
        if (pt_index >= 1024) {
            pt_index = 0;
            pdir_index++;
            ptable_virt.rebind(pdir_virt.get()->entries[pdir_index].pt_base * PAGE_SIZE);
        }
    }
}

void VMM::psized_allocate_space_from(uint32_t pdir_phys, uint32_t page, uint32_t pages, uint32_t flags)
{
    auto pdir_virt = PageBinder<PageDir*>(pdir_phys, m_buffer_1);

    uint32_t pdir_index = page / 1024;
    uint32_t pt_index = page % 1024;

    create_ptable_if_neccesary(pdir_virt.get()->entries[pdir_index], flags);

    auto ptable_virt = PageBinder<PageTable*>(pdir_virt.get()->entries[pdir_index].pt_base * PAGE_SIZE, m_buffer_2);

    for (; pages > 0; pages--, page++) {
        if (!ptable_virt.get()->entries[pt_index]._bits) {
            ptable_virt.get()->entries[pt_index].frame_adress = PMM::the().allocate_frame();
            ptable_virt.get()->entries[pt_index]._bits |= (flags & 0x7);
        }
        pt_index++;
        if (pt_index >= 1024) {
            pt_index = 0;
            pdir_index++;
            create_ptable_if_neccesary(pdir_virt.get()->entries[pdir_index], flags);
            ptable_virt.rebind(pdir_virt.get()->entries[pdir_index].pt_base * PAGE_SIZE);
        }
    }
}

KErrorOr<uint32_t> VMM::psized_allocate_space(uint32_t page_directory_phys, uint32_t pages, uint32_t flags)
{
    auto free_page_start = psized_find_free_space(page_directory_phys, pages);

    if (free_page_start) {
        psized_allocate_space_from(page_directory_phys, free_page_start.result(), pages, flags);
    }

    return free_page_start;
}

KErrorOr<uint32_t> VMM::psized_find_free_space(uint32_t page_directory_phys, uint32_t pages)
{
    auto page_dir_virt = PageBinder<PageDir*>(page_directory_phys, m_buffer_1);
    auto page_table_virt = PageBinder<PageTable*>(page_dir_virt.get()->entries[0].pt_base * 4096, m_buffer_2);

    uint32_t cur_pages = 0;
    uint32_t cur_page = 0;

    for (size_t i = 0; i < 1024 && cur_pages < pages; i++) {
        if (!page_dir_virt.get()->entries[i]._bits) {
            cur_pages += 1024;
        } else {
            page_table_virt.rebind(page_dir_virt.get()->entries[i].pt_base * 4096);
            for (size_t j = 0; j < 1024; j++) {
                if (!page_table_virt.get()->entries[j]._bits) {
                    cur_pages++;
                    if (cur_pages >= pages) {
                        break;
                    }
                } else {
                    cur_page = i * 1024 + j + 1;
                    cur_pages = 0;
                }
            }
        }
    }

    if (cur_pages < pages) {
        return KError(ENOMEM);
    }

    return cur_page;
}

void VMM::handle_interrupt(Trapframe* tf)
{
    Log() << "\nPage fault! Info:\n";

    Log() << "Virtual address: ";
    Log() << get_cr2();

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

    if (Tasking::TaskManager::s_initialized) {

        Log() << "\nPID: " << Tasking::TaskManager::the().cur_process()->id() << "\n";
        auto thread = Tasking::TaskManager::the().cur_thread();
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

void VMM::inspect_page_table(uint32_t page_table_phys, uint32_t page_table_index)
{
    auto page_table_virt = PageBinder<PageTable*>(page_table_phys, m_buffer_1);

    for (size_t i = 0; i < 1024; i++) {
        if (page_table_virt.get()->entries[i]._bits) {
            Log() << "Page: " << (page_table_index * 1024 + i) * 4096 << " , Frame: " << page_table_virt.get()->entries[i].frame_adress * 4096 << "\n";
        }
    }
}

}