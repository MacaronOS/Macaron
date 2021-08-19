#include <Libkernel/Assert.hpp>
#include <Libkernel/Graphics/VgaTUI.hpp>

void ASSERT_PANIC(char* message)
{
    VgaTUI::Print("ASSERT PANIC:\n");
    VgaTUI::Print(message);
    VgaTUI::Print("\n");
    while (1) { }
}

void STOP()
{
    while (1) { }
}