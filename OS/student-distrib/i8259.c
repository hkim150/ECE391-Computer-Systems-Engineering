#include "i8259.h"

/*
* void i8259_init()
*   input: none
*   output: none
*	description: initialize master and slave PIC's
*/
void i8259_init(){
	outb(MASK_ALL, MASTER_DATA);		// mask interrupts
	outb(MASK_ALL, SLAVE_DATA);

	outb(ICW1, MASTER_PORT);
	outb(ICW1, SLAVE_PORT);

	outb(ICW2_M, MASTER_DATA);			// save starting address of IDT
	outb(ICW2_S, SLAVE_DATA);

	outb(ICW2_M, MASTER_DATA);			// slave PIC connected to IRQ2 on master PIC
	outb(ICW3_S, SLAVE_DATA);

	outb(ICW4, MASTER_DATA);
	outb(ICW4, SLAVE_DATA);

	outb(MASK_ALL, MASTER_DATA);		// mask all interrupts
	outb(MASK_ALL, SLAVE_DATA);
}

/*
* void enable_irq(unsigned int irq_num)
*   input: interrupt request number
*   output: none
*	description: unmask input IRQ and enable interrupt
*/
void enable_irq(unsigned int irq_num){
	unsigned char int_mask_reg, new_int_mask_reg;
	if(irq_num < IRQ_MAX_NUM){							// clear corresponding bit in int_mask_reg
		int_mask_reg = inb(MASTER_DATA);
		new_int_mask_reg = ~(~int_mask_reg | 1 << irq_num);
		outb(new_int_mask_reg, MASTER_DATA);
	}
	else{												// int_mask_reg update
		int_mask_reg = inb(SLAVE_DATA);
		new_int_mask_reg = ~(~int_mask_reg | 1 << (irq_num - IRQ_MAX_NUM));
		outb(new_int_mask_reg, SLAVE_DATA);
	}
}

/*
* void disable_irq(unsigned int irq_num)
*   input: interrupt request number
*   output: none
*	description: mask input IRQ and disable interrupt
*/
void disable_irq(unsigned int irq_num){
	unsigned char int_mask_reg;

	if(irq_num < IRQ_MAX_NUM){						// check if the irq_num is in master PIC
		int_mask_reg = inb(MASTER_PORT);
		outb(int_mask_reg | 1 << irq_num, MASTER_DATA);
	}
	else{
		int_mask_reg = inb(SLAVE_DATA);
		outb(int_mask_reg | 1 << (irq_num - IRQ_MAX_NUM), SLAVE_DATA);
	}
}

/*
* void send_eoi(unsigned int irq_num)
*   input: interrupt request number
*   output: none
*	description: send end of interrupt for the interrupt request
*/
void send_eoi(unsigned int irq_num){
	int slave_irq_num;

	if (irq_num < IRQ_MAX_NUM){outb(EOI | irq_num, MASTER_PORT);}	// check if master sent irq
	else{ 										// if slave, send end of interrupt to both PIC's
		slave_irq_num = irq_num - IRQ_MAX_NUM;
		outb(EOI | slave_irq_num, SLAVE_PORT);
		outb(EOI | SLAVE_NUM, MASTER_PORT);
	}
}

