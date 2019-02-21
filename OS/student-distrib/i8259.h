#ifndef __I8259_H__
#define __I8259_H__

#include "types.h"
#include "lib.h"

#define MASTER_PORT  	0x20
#define MASTER_DATA		0x21
#define SLAVE_PORT   	0xA0
#define SLAVE_DATA 		0xA1
#define IRQ_MAX_NUM 	8			// max irq number in PIC
#define SLAVE_NUM 		2
#define MASK_ALL 		0xFF

#define EOI             0x60
#define ICW1    		0x11
#define ICW2_M  		0x20
#define ICW2_S		   	0x28
#define ICW3_M		  	0x04
#define ICW3_S		   	0x02
#define ICW4         	0x01


void i8259_init();							// master and slave PIC initialize
void enable_irq(unsigned int irq_num);		// unmask the input irq
void disable_irq(unsigned int irq_num);		// mask the input irq
void send_eoi(unsigned int irq_num);		// send end of interrupt signal for the input irq

#endif

