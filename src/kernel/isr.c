#include "isr.h"
#include "monitor.h"
#include "port.h"

isr_handler_t interrupt_handlers[256];

void register_interrupt_handler(uint8_t n, isr_handler_t handler)
{  
   interrupt_handlers[n] = handler;
}

// This gets called from our ASM interrupt handler stub.
void isr_handler(registers_t regs)
{
   if (interrupt_handlers[regs.int_no] != 0) {
      interrupt_handlers[regs.int_no]();
   } 
   else {
      term_print("recieved interrupt: ");
      term_printd(regs.int_no);
      term_print("\n");
   }
}

// This gets called from our ASM interrupt handler stub.
void irq_handler(registers_t regs)
{
   // Send an EOI (end of interrupt) signal to the PICs.
   // If this interrupt involved the slave.
   if (regs.int_no >= 40)
   {
       // Send reset signal to slave.
       outb(0xA0, 0x20);
   }
   // Send reset signal to master. (As well as slave, if necessary).
   outb(0x20, 0x20);

   if (interrupt_handlers[regs.int_no] != 0)
   {
       interrupt_handlers[regs.int_no](regs);
   }
}