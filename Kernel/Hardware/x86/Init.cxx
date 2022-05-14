#include "../Init.hpp"
#include "DescriptorTables/GDT.hpp"
#include "DescriptorTables/IDT.hpp"

namespace Kernel {

void initialize_hardware()
{
    DescriptorTables::GDT::Setup();
    DescriptorTables::IDT::Setup();
}

}