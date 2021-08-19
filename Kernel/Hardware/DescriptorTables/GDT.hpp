#pragma once

#include <Wisterialib/common.hpp>

namespace Kernel::DescriptorTables::GDT {

constexpr uint8_t Null = 0;
constexpr uint8_t KernelCode = 1;
constexpr uint8_t KernelData = 2;
constexpr uint8_t UserCode = 3;
constexpr uint8_t UserData = 4;
constexpr uint8_t TSS = 5;

constexpr uint8_t GDTEntrySize = 0x8;

constexpr uint8_t KernelCodeOffset = KernelCode * GDTEntrySize;
constexpr uint8_t KernelDataOffset = KernelData * GDTEntrySize;
constexpr uint8_t UserCodeOffset = UserCode * GDTEntrySize;
constexpr uint8_t UserDataOffset = UserData * GDTEntrySize;
constexpr uint8_t TSSOfset = TSS * GDTEntrySize;

constexpr uint8_t RequestRing3 = 3;

void Setup();
void SetKernelStack(uint32_t stack);

}