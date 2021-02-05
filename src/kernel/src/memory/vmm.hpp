#pragma once
#include "regions.hpp"

#include "../types.hpp"
#include "../assert.hpp"
#include "../algo/Singleton.hpp"
#include "../algo/Singleton.hpp"

#define PAGE_SIZE 4096

typedef union {
    struct {
        uint32_t present : 1;
        uint32_t rw : 1;
        uint32_t user_mode : 1;
        uint32_t reserved_1 : 2;
        uint32_t access : 1;
        uint32_t dirty : 1;
        uint32_t reserved_2 : 2;
        uint32_t available : 3;
        uint32_t frame_adress : 20;
    };
    uint32_t __bits;
} page_table_entry_t;

typedef struct {
    page_table_entry_t entries[1024];
} page_table_t;

typedef union {
    struct {
        uint32_t present : 1;
        uint32_t rw : 1;
        uint32_t user_mode : 1;
        uint32_t write_through : 1;
        uint32_t cache : 1; // if set, page table will be cached
        uint32_t access : 1;
        uint32_t reserved : 1;
        uint32_t page_size : 1; // 0 - 4kb, 1 - 4mb
        uint32_t global_page : 1; // ignored
        uint32_t available : 3;
        uint32_t page_table_base_adress : 20;
    };
    uint32_t __bits;
} page_directory_entry_t;

typedef struct {
    page_directory_entry_t entries[1024];
} page_directory_t;

class VMM : public Singleton<VMM> {
public:
    VMM()
        : m_buffer_1(get_pd_temp_location())
        , m_buffer_2(get_pt_temp_location())
    {
    }

    uint32_t kernel_page_directory() const { return m_kernel_directory_phys; }

    void set_page_directory(uint32_t page_directory_phys);

    void create_frame(uint32_t page_directory_phys, uint32_t frame_virt_addr);

    uint32_t clone_page_directory(uint32_t src_page_directory_phys = 0);

private:
    uint32_t create_page_table();

    // clone helping functions
    uint32_t clone_page_table(uint32_t src_page_table_phys);
    uint32_t clone_frame(uint32_t src_frame_phys);

    // buffers functions
    void map_to_buffer(uint32_t phys, uint32_t buf);
    void unmap_from_buffer(uint32_t buf);
    uint32_t get_buffered_phys_address(uint32_t buff);

private:
    // virtual addresses of buffers
    uint32_t m_buffer_1;
    uint32_t m_buffer_2;

    uint32_t m_kernel_directory_phys { clone_page_directory(0) }; // temp decision
    uint32_t cur_directory_phys;
};
