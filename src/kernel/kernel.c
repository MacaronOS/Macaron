#include "monitor.h"
#include "descriptor_tables.h"

void kernel_main(){
    init_descriptor_tables();
    term_init();

    term_print("Hello, World!\n");
	term_print("Welcome to the kernel.\n");
    
    asm volatile ("int $0x3");
    asm volatile ("int $0x4");
}