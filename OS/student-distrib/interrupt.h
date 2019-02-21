#ifndef _INTERRUPT_H
#define _INTERRUPT_H

#include "lib.h"
#include "x86_desc.h"
#include "init_idt.h"
#include "keyboard.h"
#include "function_func.h"
#include "i8259.h"

extern void idt_setup();

void idt_insert(unsigned char idx, int type, unsigned char dpl, void *handler);

void set_trap_gate(unsigned char idx, void *handler);
void set_intr_gate(unsigned char idx, void *handler);
void set_system_gate(unsigned char idx, void *handler);

#endif
