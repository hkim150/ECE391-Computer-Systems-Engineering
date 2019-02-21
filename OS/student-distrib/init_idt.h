#ifndef _INIT_IDT_H
#define _INIT_IDT_H

#include "lib.h"

extern void divide_error_exception();
extern void debug_exception();
extern void nmi_interrupt();
extern void breakpoint_exception();
extern void overflow_exception();
extern void bound_exception();
extern void invalid_opcode_exception();
extern void device_not_available_exception();
extern void double_fault_exception();
extern void coprocessor_segment_overrun();
extern void invalid_TSS_exception();
extern void segment_not_present();
extern void stack_fault_exception();
extern void general_protection_exception();
extern void page_fault_exception();
extern void floating_point_error();
extern void alignment_check_exception();
extern void machine_check_exception();
extern void simd_coprocessor_error();

#endif
