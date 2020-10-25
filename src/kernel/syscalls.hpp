#pragma once

void syscalls_init();
extern "C" void sys_printd(int);
extern "C" void switch_to_user_mode();