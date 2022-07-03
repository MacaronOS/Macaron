#pragma once

#include <Macaronlib/Common.hpp>

namespace Kernel {

// https://developer.arm.com/documentation/ddi0438/i/system-control/register-descriptions/configuration-base-address-register
static inline uintptr_t read_peripheral_base()
{
    uint32_t cbar;
    asm("mrc p15, #4, %0, c15, c0, #0"
        : "=r"(cbar));
    return cbar & 0xffff8000;
}

static inline uintptr_t read_data_fault_address()
{
    uint32_t dfar;
    asm volatile("mrc p15, 0, %0, c6, c0, 0"
                 : "=r"(dfar));
    return dfar;
}

namespace FaultStatus {
    enum class FSR {
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

union DFSR {
    struct [[gnu::packed]] {
        uint32_t FS03 : 4;
        uint32_t domain : 4;
        uint32_t _reserved1 : 1;
        uint32_t large_physical_address_extension : 1;
        uint32_t FS4 : 1;
        uint32_t write_not_read : 1;
        uint32_t external_abort_type : 1;
        uint32_t cache_maintenance_fault : 1;
        uint32_t _reserved2 : 18;
    };

    inline uint32_t get_fault_status()
    {
        return FS03 | FS4 << 4;
    }

    inline bool is_caused_by_write()
    {
        return write_not_read == 1;
    }

    uint32_t __bits;
};

static inline DFSR read_data_fault_status()
{
    DFSR dfsr;
    asm volatile("mrc p15, 0, %0, c5, c0, 0"
                 : "=r"(dfsr.__bits));
    return dfsr;
}

static inline uintptr_t read_instruction_fault_address()
{
    uint32_t ifar;
    asm volatile("mrc p15, 0, %0, c6, c0, 1"
                 : "=r"(ifar));
    return ifar;
}

static inline uintptr_t read_instruction_fault_status()
{
    uint32_t ifar;
    asm volatile("mrc p15, 0, %0, c5, c0, 1"
                 : "=r"(ifar));
    return ifar;
}

}