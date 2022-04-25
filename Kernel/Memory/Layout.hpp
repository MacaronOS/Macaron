#pragma once
#include <Macaronlib/Common.hpp>

namespace Kernel::Memory {

/*
Layout represents an object which is responsible for keeping the initial memory layout.

Basic Layout
----0x00100000-----
multiboot - section providing booting it's inaccesible from kernel
KernelStart - where the kernel is loaded
..............
KernelStackStart - kernel stack is located in the bss section
.....1KB......
KernelStackEnd - not inclusive 
..............
KernelEnd - not inclusive 
KernelHeapStart - kernel heap is located straight after the kernel
.....4MB......
KernelHeapEnd - not inclusive
PagingBuffer1 - one of the 2 PAGE SIZED buffers, which is used for temporaly mapping in VMM
.....4KB......
PagingBuffer2 - one of the 2 PAGE SIZED buffers, which is used for temporaly mapping in VMM
.....4KB......
-------------------

It's also worth mentioning, that when kernel is being booted, it sets up paging and mapps
memory in range [0;8MB] at the addreses shifted by HIGHER_HALF_OFFSET=0xC0000000.
That's why Layout provides 2 types of inforamtion - Physical and Virtual, where Virtual is the same
as Physical, but shifted by HIGHER_HALF_OFFSET
*/

constexpr uint32_t HIGHER_HALF_OFFSET = 0xC0000000;
constexpr uint32_t HEAP_SIZE = 4 * 1024 * 1024;
constexpr uint32_t PAGING_BUFFER_SIZE = 4 * 1024;

enum class LayoutElement {
    KernelStart,
    KernelStackStart,
    KernelStackEnd,
    KernelEnd,
    KernelHeapStart,
    KernelHeapEnd,
    PagingBuffer1,
    PagingBuffer2,
};

namespace Layout {
    uint32_t GetLocationPhys(LayoutElement element);
    uint32_t GetLocationVirt(LayoutElement element);
};

}