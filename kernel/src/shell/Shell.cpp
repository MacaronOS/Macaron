#include "Shell.hpp"
#include <wisterialib/StaticStack.hpp>
#include <wisterialib/String.hpp>
#include "../drivers/DriverManager.hpp"
#include "../drivers/Keyboard.hpp"
#include "../fs/base/VNode.hpp"
#include "../fs/vfs/vfs.hpp"
#include "../posix.hpp"
#include "cmd.hpp"

namespace kernel::shell {

using namespace drivers;
using namespace fs;

static bool s_shift_pressed = false;
static bool s_caps_lock_pressed = false;
static Keyboard* s_keyboard;
static String cur_path;

static char key_to_ascii(Key key)
{
    const static char asc[] = {
        '\0', // undefined symbol
        '\0', // esc
        '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
        '-', '=',
        '\0', '\0', // backspace, tab
        'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']',
        '\0', '\0', // enter, leftctrl
        'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l',
        ';', '\'', '`',
        '\0', // left shift
        '\\',
        'z', 'x', 'c', 'v', 'b', 'n', 'm',
        ',', '.', '/',
        '\0', // right shift
        '*',
        '\0', // right alt
        ' ',
        '\0', // caps lock
        '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', // f1-f10
        '\0', '\0', // num lock, scroll lock
        '7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '4', '.',
        '\0', '\0', '\0', // undefined
        '\0', '\0', // f11, f12
    };
    return asc[static_cast<uint8_t>(key)];
}

static char apply_shift(Key key)
{
    switch (key) {
    case Key::Asterisk:
        return '~';
    case Key::Num_1:
        return '!';
    case Key::Num_2:
        return '@';
    case Key::Num_3:
        return '#';
    case Key::Num_4:
        return '$';
    case Key::Num_5:
        return '%';
    case Key::Num_6:
        return '^';
    case Key::Num_7:
        return '&';
    case Key::Num_8:
        return '*';
    case Key::Num_9:
        return '(';
    case Key::Num_0:
        return ')';
    case Key::Minus:
        return '_';
    case Key::Equal:
        return '+';
    case Key::LeftBracket:
        return '{';
    case Key::RightBracket:
        return '}';
    case Key::Backslash:
        return '|';
    case Key::Semicolon:
        return ':';
    case Key::Quote:
        return '\"';
    case Key::Comma:
        return '<';
    case Key::Dot:
        return '>';
    case Key::Slash:
        return '?';
    }

    return '\0';
}

static String get_absolute_path(const String& path = "")
{
    if (!path.size()) {
        return cur_path;
    }
    if (path[0] == '/') {
        return path;
    }

    if (cur_path == "/") {
        return cur_path + path;
    }
    return cur_path + String("/") + path;
}

static bool check_path_exists(const String& path)
{
    auto fd = VFS::the().open(path, 11);
    if (!fd) {
        return false;
    }
    VFS::the().close(fd.result());
    return true;
}

static void handle_cmd_cd(const String& path)
{
    auto cp_cur_path = cur_path;

    if (path.size() && path[0] == '/') {
        cur_path = path;
    } else {
        if (!(cur_path == "/")) {
            cur_path.push_back('/');
        }
        cur_path += path;
    }

    if (!check_path_exists(cur_path)) {
        cur_path = cp_cur_path;
        term_print("path does not exist");
    }
}

KeyboardEvent get_keyboard_event()
{
    KeyboardEvent event = {};

    while ((event = s_keyboard->last_keybord_event()).key == Key::Undefined) { }
    s_keyboard->discard_last_keyboard_event();

    return event;
}

String enter_command()
{
    term_print(cur_path);
    term_print(": ");

    String command = "";
    KeyboardEvent event = {};

    while (true) {
        event = get_keyboard_event();
        if (event.pressed) {
            switch (event.key) {
            case Key::Enter: {
                term_print("\n");
                return command;
            }
            case Key::LeftShift:
            case Key::RightShift: {
                s_shift_pressed = true;
                break;
            }
            case Key::Backspace: {
                if (command.size()) {
                    command.pop_back();
                    dec_vga_pos();
                }
                break;
            }

            default: {
                char c = key_to_ascii(event.key);
                if (s_shift_pressed) {
                    char shifted = apply_shift(event.key);
                    if (shifted == '\0') {
                        c += 'A' - 'a';
                    } else {
                        c = shifted;
                    }
                }
                command.push_back(c);
                term_putc(c);
            }
            }
        } else {
            switch (event.key) {
            case Key::LeftShift:
            case Key::RightShift: {
                s_shift_pressed = false;
                break;
            }
            }
        }
    }

    return command;
}

static void handle_cmd_clear()
{
    term_init();
}

static void execute_command(const String& command)
{
    auto splitted_command = command.split(" ");

    if (!splitted_command.size()) {
        term_print("incorrect input");
    } else if (splitted_command[0] == "cd" && splitted_command.size() == 2) {
        handle_cmd_cd(splitted_command[1]);
    } else if (splitted_command[0] == "clear" && splitted_command.size() == 1) {
        kernel::shell::cmd::clear();
    } else if (splitted_command[0] == "ls" && splitted_command.size() == 1) {
        kernel::shell::cmd::ls(get_absolute_path());
    } else if (splitted_command[0] == "cat" && splitted_command.size() == 2) {
        kernel::shell::cmd::cat(get_absolute_path(splitted_command[1]));
    } else if (splitted_command[0] == "echo" && splitted_command.size() == 4) {
        kernel::shell::cmd::echo(splitted_command[1], get_absolute_path(splitted_command[3]), splitted_command[2] == ">>");
    } else if (splitted_command[0] == "touch" && splitted_command.size() == 2) {
        kernel::shell::cmd::touch(get_absolute_path(splitted_command[1]));
    }
}

void run()
{
    term_init();
    term_print("Wisteria Shell v0.1\n");

    s_keyboard = static_cast<Keyboard*>(DriverManager::the().get_driver(DriverEntity::Keyboard));
    kernel::shell::cmd::init();
    cur_path = "/";

    while (true) {
        auto command = enter_command();
        execute_command(command);
        term_print("\n");
    }
}

}