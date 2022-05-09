#pragma once

#include <Hardware/CPU.hpp>

namespace Kernel::Memory {

static constexpr auto bytes_to_pages(auto bytes)
{
    return (bytes + CPU::page_size() - 1) / CPU::page_size();
}

static constexpr auto address_to_page(auto address)
{
    return address / CPU::page_size();
}

static constexpr auto page_to_address(auto page)
{
    return page * CPU::page_size();
}

}