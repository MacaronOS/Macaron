#include "vmm.hpp"
#include "Layout.hpp"
#include "memory.hpp"
#include "pagingstructs.hpp"
#include "pmm.hpp"

#include <Logger.hpp>
#include <assert.hpp>
#include <monitor.hpp>
#include <types.hpp>

#include <errors/KError.hpp>
#include <multitasking/TaskManager.hpp>

namespace kernel::memory {

using namespace Logger;

template <>
VMM* Singleton<VMM>::s_t = nullptr;
template <>
bool Singleton<VMM>::s_initialized = false;

VMM::VMM()
    : InterruptHandler(14)
    , m_buffer_1(Layout::GetLocationVirt(LayoutElement::PagingBuffer1))
    , m_buffer_2(Layout::GetLocationVirt(LayoutElement::PagingBuffer2))
{
    set_page_directory(m_kernel_directory_phys);
}

void VMM::set_page_directory(uint32_t page_directory_phys)
{
    set_cr3(page_directory_phys);
}

void VMM::create_frame(uint32_t page_directory_phys, uint32_t frame_virt_addr)
{
    auto page_directory_virt = PageBinder<page_directory_t*>(page_directory_phys, m_buffer_1);

    if (!page_directory_virt.get()->entries[frame_virt_addr / PAGE_SIZE / 1024].__bits) {
        page_directory_virt.get()->entries[frame_virt_addr / PAGE_SIZE / 1024].page_table_base_adress = create_page_table() / PAGE_SIZE;
        page_directory_virt.get()->entries[frame_virt_addr / PAGE_SIZE / 1024].present = true;
        page_directory_virt.get()->entries[frame_virt_addr / PAGE_SIZE / 1024].rw = true;
        page_directory_virt.get()->entries[frame_virt_addr / PAGE_SIZE / 1024].user_mode = true;
    }

    uint32_t page_table_phys = page_directory_virt.get()->entries[frame_virt_addr / PAGE_SIZE / 1024].page_table_base_adress * PAGE_SIZE;
    auto page_table_virt = PageBinder<page_table_t*>(page_table_phys, m_buffer_1);

    if (page_table_virt.get()->entries[frame_virt_addr / PAGE_SIZE % 1024].__bits) {
        return;
    }

    page_table_virt.get()->entries[frame_virt_addr / PAGE_SIZE % 1024].frame_adress = PMM::the().allocate_frame();
    page_table_virt.get()->entries[frame_virt_addr / PAGE_SIZE % 1024].present = true;
    page_table_virt.get()->entries[frame_virt_addr / PAGE_SIZE % 1024].rw = true;
    page_table_virt.get()->entries[frame_virt_addr / PAGE_SIZE % 1024].user_mode = true;
}

void VMM::clear_user_directory_pages(uint32_t src_page_directory_phys)
{
    auto page_dir_virt = PageBinder<page_directory_t*>(src_page_directory_phys, m_buffer_2);

    for (size_t i = 0; i < 1024; i++) {
        if (i == 768 || i == 769) {
            continue; // skip kernel page tables
        }
        if (page_dir_virt.get()->entries[i].__bits) {
            clear_user_table_pages(page_dir_virt.get()->entries[i].page_table_base_adress * 4096);
        }
    }
}

uint32_t VMM::clone_page_directory(uint32_t src_page_directory_phys)
{
    if (!src_page_directory_phys) {
        src_page_directory_phys = (uint32_t)&boot_page_directory - HIGHER_HALF_OFFSET;
    }

    uint32_t dest_page_dir_phys = PMM::the().allocate_frame() * FRAME_SIZE;

    auto dest_page_dir_virt = PageBinder<page_directory_t*>(dest_page_dir_phys, m_buffer_1);
    auto src_page_dir_virt = PageBinder<page_directory_t*>(src_page_directory_phys, m_buffer_2);

    memset(dest_page_dir_virt.get(), 0, sizeof(page_directory_t));

    // map kernel tables to the same location
    dest_page_dir_virt.get()->entries[768] = src_page_dir_virt.get()->entries[768];
    dest_page_dir_virt.get()->entries[769] = src_page_dir_virt.get()->entries[769];

    for (size_t i = 0; i < 768; i++) {
        if (src_page_dir_virt.get()->entries[i].__bits) {
            dest_page_dir_virt.get()->entries[i] = src_page_dir_virt.get()->entries[i];
            dest_page_dir_virt.get()->entries[i].page_table_base_adress = clone_page_table(src_page_dir_virt.get()->entries[i].page_table_base_adress * PAGE_SIZE) / PAGE_SIZE;
        }
    }

    return dest_page_dir_phys;
}

KErrorOr<uint32_t> VMM::allocate_space(uint32_t page_directory_phys, uint32_t size)
{
    auto page_dir_virt = PageBinder<page_directory_t*>(page_directory_phys, m_buffer_1);

    const uint32_t size_in_pages = (size + PAGE_SIZE - 1) / PAGE_SIZE;

    uint32_t cur_pages = 0;
    uint32_t cur_page = 0;

    for (size_t i = 0; i < 1024 && cur_pages < size_in_pages; i++) {
        if (!page_dir_virt.get()->entries[i].__bits) {
            cur_pages += 1024;
        } else {
            auto page_table_virt = PageBinder<page_table_t*>(page_dir_virt.get()->entries[i].page_table_base_adress * 4096, m_buffer_2);
            for (size_t j = 0; j < 1024; j++) {
                if (!page_table_virt.get()->entries[j].__bits) {
                    cur_pages++;
                    if (cur_pages >= size_in_pages) {
                        break;
                    }
                } else {
                    cur_page = i * 1024 + j;
                    cur_pages = 0;
                }
            }
        }
    }

    if (cur_pages < size_in_pages) {
        return KError(ENOMEM);
    }

    for (size_t page = cur_page; page < cur_page + size_in_pages; page++) {
        create_frame(page_directory_phys, page * PAGE_SIZE);
    }

    return cur_page * PAGE_SIZE;
}

uint32_t VMM::create_page_table()
{
    uint32_t page_table_phys = PMM::the().allocate_frame() * FRAME_SIZE;
    auto page_table_virt = PageBinder<page_table_t*>(page_table_phys, m_buffer_1);

    memset(page_table_virt.get(), 0, sizeof(page_table_t));

    return page_table_phys;
}

uint32_t VMM::clone_page_table(uint32_t src_page_table_phys)
{
    // allocate space for the new page table
    uint32_t dest_page_table_phys = PMM::the().allocate_frame() * FRAME_SIZE;

    auto dest_page_table_virt = PageBinder<page_table_t*>(dest_page_table_phys, m_buffer_1);
    auto src_page_table_virt = PageBinder<page_table_t*>(src_page_table_phys, m_buffer_2);

    memset(dest_page_table_virt.get(), 0, sizeof(page_table_t));

    for (size_t i = 0; i < 1024; i++) {
        if (src_page_table_virt.get()->entries[i].__bits) {
            dest_page_table_virt.get()->entries[i] = src_page_table_virt.get()->entries[i];
            dest_page_table_virt.get()->entries[i].frame_adress = clone_frame(src_page_table_virt.get()->entries[i].frame_adress * PAGE_SIZE) / PAGE_SIZE;
        }
    }

    return dest_page_table_phys;
}

uint32_t VMM::clone_frame(uint32_t src_frame_phys)
{
    uint32_t dest_frame_phys = PMM::the().allocate_frame() * FRAME_SIZE;

    auto src_page = PageBinder<void*>(src_frame_phys, m_buffer_1);
    auto dest_page = PageBinder<void*>(dest_frame_phys, m_buffer_2);

    memcpy(dest_page.get(), src_page.get(), PAGE_SIZE);

    return dest_frame_phys;
}

void VMM::clear_user_table_pages(uint32_t src_page_table_phys)
{
    auto page_table_virt = PageBinder<page_table_t*>(src_page_table_phys, m_buffer_1);
    for (int i = 0; i < 1024; i++) {
        PMM::the().free_frame(page_table_virt.get()->entries[i].frame_adress);
        page_table_virt.get()->entries[i].__bits = 0;
    }
}

void VMM::handle_interrupt(trapframe_t* tf)
{
    term_print("\nPage fault! Info:\n");

    term_print("Virtual address: ");
    term_printd(get_cr2());

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

    term_print("\nFlags: ");

    for (auto flag : flags) {
        if (tf->err_code & (1 << static_cast<uint32_t>(flag))) {
            term_print(descr[static_cast<uint32_t>(flag)]);
            term_print(" ");
        }
    }

    term_print("\nPID:");
    term_printd(multitasking::TaskManager::the().cur_thread()->process->id);
    term_print("\n");

    STOP();
}

void VMM::inspect_page_diriectory(uint32_t page_directory_phys)
{
    auto page_dir_virt = PageBinder<page_directory_t*>(page_directory_phys, m_buffer_2);

    for (size_t i = 0; i < 1024; i++) {
        if (i == 768 || i == 769) {
            continue; // skip kernel page tables
        }
        if (page_dir_virt.get()->entries[i].__bits) {
            inspect_page_table(page_dir_virt.get()->entries[i].page_table_base_adress * 4096, i);
        }
    }
}

void VMM::inspect_page_table(uint32_t page_table_phys, uint32_t page_table_index)
{
    auto page_table_virt = PageBinder<page_table_t*>(page_table_phys, m_buffer_1);

    for (size_t i = 0; i < 1024; i++) {
        if (page_table_virt.get()->entries[i].__bits) {
            Log() << "Page: " << (page_table_index * 1024 + i) * 4096 << " , Frame: " << page_table_virt.get()->entries[i].frame_adress * 4096 << "\n";
        }
    }
}

}