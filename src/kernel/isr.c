#include "isr.h"
#include "monitor.h"

// This gets called from our ASM interrupt handler stub.
void isr_handler(registers_t regs)
{
   term_print("recieved interrupt: ");
   term_printd(regs.int_no);
   term_print("\n");
}