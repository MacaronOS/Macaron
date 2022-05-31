#pragma once

#include <Macaronlib/Common.hpp>

using Descriptor = uint32_t;

struct [[gnu::packed]] FirstLevelTranslationTable {
    Descriptor descriptors[4096];
};

struct [[gnu::packed]] SecondLevelTranslationTable {
    Descriptor descriptors[256];
};

// Descriptors AP flags
// https://developer.arm.com/documentation/ddi0406/b/System-Level-Architecture/Virtual-Memory-System-Architecture--VMSA-/Memory-access-control/Access-permissions
enum class DescrAP {
    KernelWrite = 0b00,
    UserWrite = 0b01,
    KernelRead = 0b10,
    UserRead = 0b11,
};

enum class DescrExec {
    Allow = false,
    Restrict = true,
};

// ------------------------ Short-descriptor translation table descriptors ------------------------
// https://developer.arm.com/documentation/ddi0406/cb/System-Level-Architecture/Virtual-Memory-System-Architecture--VMSA-/Short-descriptor-translation-table-format/Short-descriptor-translation-table-format-descriptors?lang=en
union Section {
    Section(uint32_t section_base_address, DescrAP access_permissions, DescrExec exec_permissions)
    {
        __bits = 0;
        always_one = 1;
        always_zero = 0;
        base_address = section_base_address & 0xfff;
        AP0 = 1; // Use simplified access permissions model
        AP1 = static_cast<uint32_t>(access_permissions) & 0x1; // Extract the first bit
        AP2 = (static_cast<uint32_t>(access_permissions) >> 1) & 0x1; // Extract the second bit
        PXN = static_cast<bool>(exec_permissions);
        XN = static_cast<bool>(exec_permissions);
    };

    Section(uint32_t descriptor_entry)
    {
        __bits = descriptor_entry;
    }

    bool match()
    {
        return always_one && !always_zero;
    }

    static Section* from(Descriptor* descriptor_entry)
    {
        return (Section*)(void*)(descriptor_entry);
    }

    void set_access_permissions(DescrAP access_permissions)
    {
        AP0 = 1; // Use simplified access permissions model
        AP1 = static_cast<uint32_t>(access_permissions) & 0x1; // Extract the first bit
        AP2 = (static_cast<uint32_t>(access_permissions) >> 1) & 0x1; // Extract the second bit
    }

    struct [[gnu::packed]] {
        uint32_t PXN : 1; // Privileged Execute Never
        uint32_t always_one : 1; // Mark as section / supersection
        uint32_t B : 1; // Bufferable
        uint32_t C : 1; // Cacheable
        uint32_t XN : 1; // Execute Never
        uint32_t dommain : 4;
        uint32_t implementation_defined : 1;
        uint32_t AP0 : 1; // Access permissions bit [0]
        uint32_t AP1 : 1; // Access permissions bit [1]
        uint32_t TEX : 3; // TEX[2:0]
        uint32_t AP2 : 1; // Access permissions bit [2]
        uint32_t S : 1; // Shareable
        uint32_t nG : 1; // Not global. Determines how this is marked in the TLB
        uint32_t always_zero : 1; // Marks as section
        uint32_t NS : 1; // Non secure bit
        uint32_t base_address : 12; // Section base address
    };
    uint32_t __bits;
};

union Supersection {
    Supersection(uint32_t supersection_base_address, DescrAP access_permissions, DescrExec exec_permissions)
    {
        __bits = 0;
        always_one_1 = 1;
        always_one_2 = 1;
        base_address = supersection_base_address & 0xff;
        AP0 = 1; // Use simplified access permissions model
        AP1 = static_cast<uint32_t>(access_permissions) & 0x1; // Extract the first bit
        AP2 = (static_cast<uint32_t>(access_permissions) >> 1) & 0x1; // Extract the second bit
        PXN = static_cast<bool>(exec_permissions);
        XN = static_cast<bool>(exec_permissions);
    };

    Supersection(uint32_t descriptor_entry)
    {
        __bits = descriptor_entry;
    }

    bool match()
    {
        return always_one_1 && always_one_2;
    }

    static Supersection* from(Descriptor* descriptor_entry)
    {
        return (Supersection*)(void*)(descriptor_entry);
    }

    void set_access_permissions(DescrAP access_permissions)
    {
        AP0 = 1; // Use simplified access permissions model
        AP1 = static_cast<uint32_t>(access_permissions) & 0x1; // Extract the first bit
        AP2 = (static_cast<uint32_t>(access_permissions) >> 1) & 0x1; // Extract the second bit
    }

    struct [[gnu::packed]] {
        uint32_t PXN : 1; // Privileged Execute Never
        uint32_t always_one_1 : 1; // Mark as section / supersection
        uint32_t B : 1; // Bufferable
        uint32_t C : 1; // Cacheable
        uint32_t XN : 1; // Execute Never.
        uint32_t extended_base_address_39_36 : 4; // Extended base address PA[39:36]
        uint32_t implementation_defined : 1;
        uint32_t AP0 : 1; // Access permissions bit [0]
        uint32_t AP1 : 1; // Access permissions bit [1]
        uint32_t TEX : 3; // TEX[2:0]
        uint32_t AP2 : 1; // Access permissions bit [2]
        uint32_t S : 1; // Shareable
        uint32_t nG : 1; // Not global. Determines how this is marked in the TLB
        uint32_t always_one_2 : 1; // Marks as supersection
        uint32_t NS : 1; // Non secure bit
        uint32_t extended_base_address_35_32 : 4; // Extended base address PA[35:32]
        uint32_t base_address : 8; // Superection base address
    };
    uint32_t __bits;
};

union PageTable {
    PageTable(uint32_t page_table_base_address, DescrExec exec_permissions)
    {
        __bits = 0;
        always_one = 1;
        always_zero = 0;
        SBZ = 0;
        PXN = static_cast<bool>(exec_permissions);
        base_address = page_table_base_address & 0x3fffff;
    };

    PageTable(Descriptor descriptor_entry)
    {
        __bits = descriptor_entry;
    }

    bool match()
    {
        return always_one && !always_zero;
    }

    static PageTable* from(Descriptor* descriptor_entry)
    {
        return (PageTable*)(void*)(descriptor_entry);
    }

    struct [[gnu::packed]] {
        uint32_t always_one : 1; // Mark as a page table
        uint32_t always_zero : 1; // Mark as a page table
        uint32_t PXN : 1; // Privileged Execute Never
        uint32_t NS : 1; // Non secure bit
        uint32_t SBZ : 1; // Should be zero
        uint32_t dommain : 4;
        uint32_t implementation_defined : 1;
        uint32_t base_address : 22; // Page table base address
    };
    uint32_t __bits;
};

union SmallPage {
    SmallPage(uint32_t small_page_base_address, DescrAP access_permissions, DescrExec exec_permissions)
    {
        __bits = 0;
        always_one = 1;
        base_address = small_page_base_address & 0xfffff;
        AP0 = 1; // Use simplified access permissions model
        AP1 = static_cast<uint32_t>(access_permissions) & 0x1; // Extract the first bit
        AP2 = (static_cast<uint32_t>(access_permissions) >> 1) & 0x1; // Extract the second bit
        XN = static_cast<bool>(exec_permissions);
    };

    SmallPage(uint32_t descriptor_entry)
    {
        __bits = descriptor_entry;
    }

    bool match()
    {
        return always_one;
    }

    static SmallPage* from(Descriptor* descriptor_entry)
    {
        return (SmallPage*)(void*)(descriptor_entry);
    }

    void set_access_permissions(DescrAP access_permissions)
    {
        AP0 = 1; // Use simplified access permissions model
        AP1 = static_cast<uint32_t>(access_permissions) & 0x1; // Extract the first bit
        AP2 = (static_cast<uint32_t>(access_permissions) >> 1) & 0x1; // Extract the second bit
    }

    struct [[gnu::packed]] {
        uint32_t XN : 1; // Execute Never
        uint32_t always_one : 1; // Mark as a small page
        uint32_t B : 1; // Bufferable
        uint32_t C : 1; // Cacheable
        uint32_t AP0 : 1; // Access permissions bit [0]
        uint32_t AP1 : 1; // Access permissions bit [1]
        uint32_t TEX : 3; // TEX[2:0]
        uint32_t AP2 : 1; // Access permissions bit [2]
        uint32_t S : 1; // Shareable
        uint32_t nG : 1; // Not global. Determines how this is marked in the TLB
        uint32_t base_address : 20; // Small page base address
    };
    uint32_t __bits;
};
