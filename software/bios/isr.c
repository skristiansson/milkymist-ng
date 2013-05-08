#include <hw/csr.h>
#include <irq.h>
#include <uart.h>
#include <spr-defs.h>
#include <system.h>

/* move this to irq.h (?) */
#define EXTERNAL_IRQ	0x800

void isr(void);
void isr(void)
{
	unsigned int irqs;

	irqs = irq_pending() & irq_getmask();

	if(irqs & (1 << UART_INTERRUPT))
		uart_isr();
}


void exception_handler(unsigned long vect, unsigned long *sp);
void exception_handler(unsigned long vect, unsigned long *sp)
{
	if ((vect & 0xf00) == EXTERNAL_IRQ) {
		isr();
	} else {
		/* Unhandled exception */
		for(;;);
	}
}
