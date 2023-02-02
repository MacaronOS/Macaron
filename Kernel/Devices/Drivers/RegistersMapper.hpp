#pragma once

#include <Devices/Device.hpp>
#include <Devices/DeviceManager.hpp>
#include <Devices/Drivers/Driver.hpp>

#include <Memory/VMM/VMM.hpp>
#include <Tasking/MemoryDescription/MemoryDescription.hpp>
#include <Tasking/MemoryDescription/SharedVMArea.hpp>

#include <Macaronlib/Tuple.hpp>

namespace Kernel::Devices {

using namespace Tasking;

template <uintptr_t PhysicalAddress, typename RegistersMask>
struct RegistersDescription {
    using RegistersMaskType = RegistersMask;
    static constexpr size_t registers_size = sizeof(RegistersMask);
    static constexpr uintptr_t physical_address = PhysicalAddress;

    volatile RegistersMask* register_mask;
};

template <typename TupleType, int I>
class _IthRegisterMapperHelper {
public:
    void map(TupleType& tuple)
    {
        auto& memory_mapped_description = tuple.template get<I>();

        auto registers_area = kernel_memory_description.allocate_memory_area<ExplicitlySharedVMArea>(
            memory_mapped_description.registers_size,
            VM_READ | VM_WRITE,
            HIGHER_HALF_OFFSET);

        if (!registers_area) {
            ASSERT_PANIC("[UART] Could not allocate uart vmarea");
        }

        registers_area.result()->set_pm_start(memory_mapped_description.physical_address);
        auto registers_area_virtual_address = registers_area.result()->vm_start();
        memory_mapped_description.register_mask = reinterpret_cast<TupleElement<I, TupleType>::Type::RegistersMaskType*>(
            registers_area_virtual_address);

        VMM::the().map_memory(
            registers_area_virtual_address,
            memory_mapped_description.physical_address,
            memory_mapped_description.registers_size,
            1);

        _IthRegisterMapperHelper<TupleType, I - 1>().map(tuple);
    }
};

template <typename TupleType>
class _IthRegisterMapperHelper<TupleType, -1> {
public:
    void map(TupleType& tuple)
    {
        return;
    }
};

template <typename... Descriptions>
class RegistersMapper {
public:
    void map_all_registers()
    {
        _IthRegisterMapperHelper<TupleType, sizeof...(Descriptions) - 1>().map(m_descriptions);
    }

    template <size_t I>
    auto& get_register_mask()
    {
        return m_descriptions.template get<I>().register_mask;
    }

private:
    using TupleType = Tuple<Descriptions...>;
    TupleType m_descriptions;
};

}