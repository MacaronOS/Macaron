#pragma once
#include "../algo/String.hpp"
namespace kernel::shell::cmd {

void init();

void clear();
void ls(const String& path);
void cd();
void cat(const String& path);
void echo(const String& msg, const String& path, bool append);
void touch(const String& path);

}
