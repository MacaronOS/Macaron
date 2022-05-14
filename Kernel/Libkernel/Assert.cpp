#include <Libkernel/Assert.hpp>
#include <Libkernel/Graphics/VgaTUI.hpp>
#include <Libkernel/Logger.hpp>

void ASSERT_PANIC(char* message)
{
    Kernel::Log() << "ASSERT PANIC:\n"
                  << message << "\n";
    while (1) { }
}

void STOP()
{
    while (1) { }
}