#include "MainActivity.hpp"
#include "SecondActivity.hpp"

#include <Libui/App/Application.hpp>
#include <Libui/Button.hpp>
#include <Libui/LinearLayout.hpp>
#include <Libui/TextView.hpp>

#include <Libgraphics/Bitmap.hpp>
#include <Libgraphics/Color.hpp>
#include <Libgraphics/Font/FontLoader.hpp>

MainActivity main_activity {};

int main()
{
    UI::Application::the().run();
    return 0;
}